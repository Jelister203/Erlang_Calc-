#ifndef APP_LOCALE_H
#define APP_LOCALE_H

#include <QString>
#include <QJsonObject>

/*
 * locale.h
 * Модуль локализации. Класс AppLocale (не Locale — конфликт с ::setlocale из <clocale>).
 *
 * Данные хранятся в locale_ru.json и locale_en.json.
 * Каждый файл содержит два объекта: "classic" и "latin".
 *
 * Использование:
 *   AppLocale &loc = AppLocale::instance();
 *   loc.setLanguage(AppLocale::EN);
 *   loc.setNotation(AppLocale::CLASSIC);
 *   QString s = loc.s("btn_calc");
 */

class AppLocale
{
public:
    enum Language { RU, EN };
    enum Notation { CLASSIC, LATIN };

    static AppLocale &instance();

    void setLanguage(Language lang);
    void setNotation(Notation notation);

    Language language() const { return m_lang; }
    Notation notation() const { return m_notation; }

    /* Получить строку по ключу. Возвращает ключ если не найдено. */
    QString s(const QString &key) const;

private:
    AppLocale();
    void reload();

    Language    m_lang;
    Notation    m_notation;
    QJsonObject m_strings;
};

#endif /* APP_LOCALE_H */
