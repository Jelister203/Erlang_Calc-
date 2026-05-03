/*
 * calc_core.cpp
 * Реализация расчётного ядра (см. calc_core.h).
 *
 * Все вычисления ведутся в логарифмическом масштабе
 * для численной устойчивости при больших V.
 */

#include "calc_core.h"
#include <cmath>
#include <cfloat>
#include <cstdlib>

/* ------------------------------------------------------------------ */
/* Утилиты                                                             */
/* ------------------------------------------------------------------ */

double log_factorial(int n)
{
    double result = 0.0;
    int i;
    for (i = 2; i <= n; ++i)
        result += log((double)i);
    return result;
}

/* Логарифм биномиального коэффициента ln(C(n,k)) */
static double log_binom(int n, int k)
{
    if (k < 0 || k > n) return -1e300;
    return log_factorial(n) - log_factorial(k) - log_factorial(n - k);
}

/* ------------------------------------------------------------------ */
/* 1. Модель Эрланга-Б                                                 */
/* ------------------------------------------------------------------ */

double erlang_b(int V, double A)
{
    /*
     * Рекуррентная формула (Jagerman):
     *   B(0,A) = 1
     *   B(v,A) = (A * B(v-1,A)) / (v + A * B(v-1,A))
     * Численно устойчива для любых V и A.
     */
    double B = 1.0;
    int v;
    if (V < 1 || A <= 0.0) return 0.0;
    for (v = 1; v <= V; ++v)
        B = (A * B) / ((double)v + A * B);
    return B;
}

DirectResult erlang_direct(int V, double A)
{
    DirectResult r;
    r.B = erlang_b(V, A);
    r.A_lost    = A * r.B;
    r.A_served  = A - r.A_lost;
    r.utilization = (V > 0) ? r.A_served / (double)V : 0.0;
    return r;
}

int erlang_inverse_V(double A, double B_norm)
{
    int V;
    if (A <= 0.0 || B_norm <= 0.0 || B_norm >= 1.0) return -1;
    for (V = 1; V <= 10000; ++V)
        if (erlang_b(V, A) <= B_norm) return V;
    return -1;
}

double erlang_inverse_A(int V, double B_norm)
{
    /*
     * Бинарный поиск по A: ищем A такое, что erlang_b(V,A) = B_norm.
     * erlang_b монотонно возрастает по A.
     */
    double lo, hi, mid, b;
    int iter;
    if (V < 1 || B_norm <= 0.0 || B_norm >= 1.0) return -1.0;
    lo = 0.0;
    hi = (double)V * 100.0;
    for (iter = 0; iter < 200; ++iter) {
        mid = (lo + hi) * 0.5;
        b = erlang_b(V, mid);
        if (b < B_norm) lo = mid;
        else            hi = mid;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi) * 0.5;
}

/* ------------------------------------------------------------------ */
/* 2. Модель Энгсета                                                   */
/* ------------------------------------------------------------------ */

double engset_b(int V, int N, double A0)
{
    /*
     * E(V,N,A0):
     *   числитель   = C(N,V) * A0^V
     *   знаменатель = sum_{k=0}^{V} C(N,k) * A0^k
     *
     * В лог-масштабе + log-sum-exp для устойчивости.
     */
    double log_A0, log_num, max_log, sum, log_den;
    double *terms;
    int k;

    if (V < 1 || N <= V || A0 <= 0.0) return 0.0;

    log_A0  = log(A0);
    log_num = log_binom(N, V) + (double)V * log_A0;

    terms = (double *)malloc((size_t)(V + 1) * sizeof(double));
    if (!terms) return -1.0;

    for (k = 0; k <= V; ++k)
        terms[k] = log_binom(N, k) + (double)k * log_A0;

    max_log = terms[0];
    for (k = 1; k <= V; ++k)
        if (terms[k] > max_log) max_log = terms[k];

    sum = 0.0;
    for (k = 0; k <= V; ++k)
        sum += exp(terms[k] - max_log);
    log_den = max_log + log(sum);

    free(terms);
    return exp(log_num - log_den);
}

DirectResult engset_direct(int V, int N, double A0)
{
    DirectResult r;
    double A_total = (double)N * A0;
    r.B = engset_b(V, N, A0);
    r.A_lost    = A_total * r.B;
    r.A_served  = A_total - r.A_lost;
    r.utilization = (V > 0) ? r.A_served / (double)V : 0.0;
    return r;
}

int engset_inverse_V(int N, double A0, double B_norm)
{
    int V;
    if (N < 2 || A0 <= 0.0 || B_norm <= 0.0 || B_norm >= 1.0) return -1;
    for (V = 1; V < N; ++V)
        if (engset_b(V, N, A0) <= B_norm) return V;
    return -1;
}

double engset_inverse_A0(int V, int N, double B_norm)
{
    double lo, hi, mid, b;
    int iter;
    if (V < 1 || N <= V || B_norm <= 0.0 || B_norm >= 1.0) return -1.0;
    lo = 0.0;
    hi = 1000.0;
    for (iter = 0; iter < 200; ++iter) {
        mid = (lo + hi) * 0.5;
        b = engset_b(V, N, mid);
        if (b < B_norm) lo = mid;
        else            hi = mid;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi) * 0.5;
}

/* ------------------------------------------------------------------ */
/* 3. Модель Эрланга-А (Erlang-C, M/M/V с ожиданием)                 */
/* ------------------------------------------------------------------ */

double erlang_c(int V, double A)
{
    /*
     * Формула Эрланга-С через рекуррентную формулу Эрланга-Б:
     *
     *   C(V,A) = erlang_b(V,A) / (1 - rho*(1 - erlang_b(V,A)))
     *
     *   где rho = A / V.
     *
     * Это алгебраически эквивалентно прямой формуле, но численно устойчиво
     * при любых V и A < V, поскольку использует уже отлаженный erlang_b.
     *
     * При rho >= 1 (A >= V) система неустойчива → возвращаем -1.0.
     */
    double rho, B;
    if (V < 1 || A <= 0.0) return 0.0;
    rho = A / (double)V;
    if (rho >= 1.0) return -1.0;   /* неустойчивый режим */
    B = erlang_b(V, A);
    return B / (1.0 - rho * (1.0 - B));
}

ErlangCResult erlang_c_direct(int V, double A)
{
    /*
     * Характеристики системы M/M/V (время нормировано на h = 1/mu = 1):
     *
     *   C      = erlang_c(V, A)        — вероятность ожидания
     *   rho    = A / V                 — загрузка одного канала
     *   Lq     = C * rho / (1 - rho)  — среднее число в очереди
     *   Wq     = Lq / A               — среднее время ожидания в очереди
     *             (по формуле Литтла: Lq = A * Wq)
     *   W      = Wq + 1               — полное среднее время в системе
     *             (1 = среднее время обслуживания при h=1)
     */
    ErlangCResult r;
    double rho, C;

    r.C = -1.0;
    r.W = r.Wq = r.utilization = r.Lq = 0.0;

    if (V < 1 || A <= 0.0) return r;

    rho = A / (double)V;
    if (rho >= 1.0) return r;   /* неустойчивый режим */

    C = erlang_c(V, A);

    r.C           = C;
    r.utilization = rho;
    r.Lq          = C * rho / (1.0 - rho);
    r.Wq          = (A > 0.0) ? r.Lq / A : 0.0;
    r.W           = r.Wq + 1.0;

    return r;
}

int erlang_c_inverse_V(double A, double C_norm)
{
    /*
     * Ищем минимальное V > A такое, что C(V,A) <= C_norm.
     * Начинаем с ceil(A)+1 — меньше нет смысла (rho >= 1).
     */
    int V;
    double c;
    if (A <= 0.0 || C_norm <= 0.0 || C_norm >= 1.0) return -1;
    V = (int)A + 1;
    for (; V <= 10000; ++V) {
        c = erlang_c(V, A);
        if (c >= 0.0 && c <= C_norm) return V;
    }
    return -1;
}

double erlang_c_inverse_A(int V, double C_norm)
{
    /*
     * Бинарный поиск по A в (0, V).
     * erlang_c монотонно возрастает по A при фиксированном V.
     */
    double lo, hi, mid, c;
    int iter;
    if (V < 1 || C_norm <= 0.0 || C_norm >= 1.0) return -1.0;
    lo = 0.0;
    hi = (double)V - 1e-6;   /* строго меньше V, иначе rho >= 1 */
    for (iter = 0; iter < 200; ++iter) {
        mid = (lo + hi) * 0.5;
        c = erlang_c(V, mid);
        if (c < 0.0 || c > C_norm) hi = mid;
        else                        lo = mid;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi) * 0.5;
}

/* ------------------------------------------------------------------ */
/* 4. Модель с резервированием                                         */
/* ------------------------------------------------------------------ */

double reservation_b(int V, int c, double A)
{
    int V_eff;
    if (V < 1 || c < 0 || c >= V || A <= 0.0) return 0.0;
    V_eff = V - c;
    if (V_eff <= 0) return 1.0;
    return erlang_b(V_eff, A);
}

DirectResult reservation_direct(int V, int c, double A)
{
    DirectResult r;
    r.B = reservation_b(V, c, A);
    r.A_lost    = A * r.B;
    r.A_served  = A - r.A_lost;
    r.utilization = (V > 0) ? r.A_served / (double)V : 0.0;
    return r;
}

int reservation_inverse_V(int c, double A, double B_norm)
{
    int V;
    if (c < 0 || A <= 0.0 || B_norm <= 0.0 || B_norm >= 1.0) return -1;
    for (V = c + 1; V <= 10000; ++V)
        if (reservation_b(V, c, A) <= B_norm) return V;
    return -1;
}

double reservation_inverse_A(int V, int c, double B_norm)
{
    int V_eff;
    if (V < 1 || c < 0 || c >= V || B_norm <= 0.0 || B_norm >= 1.0) return -1.0;
    V_eff = V - c;
    return erlang_inverse_A(V_eff, B_norm);
}

/* ------------------------------------------------------------------ */
/* 5. Модель с групповым поступлением                                  */
/* ------------------------------------------------------------------ */

double batch_b(int V, int g, double A)
{
    double rho_g, log_rho_g, log_num, max_log, sum, log_den;
    double *terms;
    int M, m;

    if (V < 1 || g < 1 || A <= 0.0) return 0.0;
    if (g > V) return 1.0;

    M       = V / g;
    rho_g   = A / (double)g;
    log_rho_g = log(rho_g);

    terms = (double *)malloc((size_t)(M + 1) * sizeof(double));
    if (!terms) return -1.0;

    for (m = 0; m <= M; ++m)
        terms[m] = (double)m * log_rho_g - log_factorial(m);

    max_log = terms[0];
    for (m = 1; m <= M; ++m)
        if (terms[m] > max_log) max_log = terms[m];

    sum = 0.0;
    for (m = 0; m <= M; ++m)
        sum += exp(terms[m] - max_log);
    log_den = max_log + log(sum);

    log_num = terms[M];
    free(terms);
    return exp(log_num - log_den);
}

DirectResult batch_direct(int V, int g, double A)
{
    DirectResult r;
    r.B = batch_b(V, g, A);
    r.A_lost    = A * r.B;
    r.A_served  = A - r.A_lost;
    r.utilization = (V > 0) ? r.A_served / (double)V : 0.0;
    return r;
}

int batch_inverse_V(int g, double A, double B_norm)
{
    int V;
    if (g < 1 || A <= 0.0 || B_norm <= 0.0 || B_norm >= 1.0) return -1;
    for (V = g; V <= 10000; ++V)
        if (batch_b(V, g, A) <= B_norm) return V;
    return -1;
}

double batch_inverse_A(int V, int g, double B_norm)
{
    double lo, hi, mid, b;
    int iter;
    if (V < g || g < 1 || B_norm <= 0.0 || B_norm >= 1.0) return -1.0;
    lo = 0.0;
    hi = (double)V * 100.0;
    for (iter = 0; iter < 200; ++iter) {
        mid = (lo + hi) * 0.5;
        b = batch_b(V, g, mid);
        if (b < B_norm) lo = mid;
        else            hi = mid;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi) * 0.5;
}
