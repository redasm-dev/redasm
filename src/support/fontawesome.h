#pragma once

#include <QFont>
#include <QIcon>

namespace fontawesome {

QFont fa_font();
QFont fa_brands_font();
QIcon icon(const QChar& code, const QColor& color);
QIcon icon(const QChar& code);
QIcon brand(const QChar& code, const QColor& color);
QIcon brand(const QChar& code);

} // namespace fontawesome

#define FA_FONT fontawesome::fa_font()
#define FAB_FONT fontawesome::fa_brands_font()
#define FA_ICON(code) fontawesome::icon(QChar{code})
#define FA_ICON_COLOR(code, color) fontawesome::icon(QChar{code}, color)
#define FAB_ICON(code) fontawesome::brand(QChar{code})
#define FAB_ICON_COLOR(code, color) fontawesome::brand(QChar{code}, color)
