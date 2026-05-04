/*
 * locale.cpp
 * Реализация AppLocale.
 */

#include "app_locale.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QDir>

AppLocale::AppLocale()
    : m_lang(RU), m_notation(CLASSIC)
{
    reload();
}

AppLocale &AppLocale::instance()
{
    static AppLocale inst;
    return inst;
}

void AppLocale::setLanguage(Language lang)
{
    if (m_lang == lang) return;
    m_lang = lang;
    reload();
}

void AppLocale::setNotation(Notation notation)
{
    if (m_notation == notation) return;
    m_notation = notation;
    reload();
}

void AppLocale::reload()
{
    QString filename = (m_lang == RU) ? "locale_ru.json" : "locale_en.json";

    /* Ищем рядом с .exe */
    QString path = QDir(QCoreApplication::applicationDirPath()).filePath(filename);

    /* Fallback: рабочая директория (запуск из Qt Creator) */
    if (!QFile::exists(path))
        path = QDir::current().filePath(filename);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();

    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    QJsonObject root = doc.object();
    QString notationKey = (m_notation == CLASSIC) ? "classic" : "latin";
    if (!root.contains(notationKey) || !root[notationKey].isObject())
        return;

    m_strings = root[notationKey].toObject();
}

QString AppLocale::s(const QString &key) const
{
    if (m_strings.contains(key))
        return m_strings[key].toString();
    return key;
}
