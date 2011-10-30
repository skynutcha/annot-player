#ifndef _CORE_SS_H
#define _CORE_SS_H

// core/ss.h
// 7/15/2011

// - QSS syntax -

#define SS_BEGIN(id)    #id "{"
#define SS_END          "}"

#define SS_NULL                 ";"
#define SS_PADDING(_len)        "padding:" #_len ";"
#define SS_NO_PADDING           SS_PADDING(0)
#define SS_TOP(_len)            "top:" #_len ";"
#define SS_BOTTOM(_len)         "bottom:" #_len ";"
#define SS_LEFT(_len)           "left:" #_len ";"
#define SS_RIGHT(_len)          "right:" #_len ";"
#define SS_IMAGE(_img)          "image:" #_img ";"
#define SS_IMAGE_URL(_url)      "image: url(" _url ");"
#define SS_BACKGROUND(_color)   "background:" #_color ";"
#define SS_BACKGROUND_COLOR(_color) "background-color:" #_color ";"
#define SS_BACKGROUND_IMAGE(_img)   "background-image:" #_img ";"
#define SS_BACKGROUND_IMAGE_URL(_url) "background-image: url(" _url ");"
#define SS_TRANSPARENT          "background:transparent;"
#define SS_COLOR(_color)        "color:" #_color ";"
#define SS_BORDER_COLOR(_color) "border-color:" #_color ";"
#define SS_NO_BORDER            SS_BORDER(0)
#define SS_WIDTH(_len)          "width:" #_len ";"
#define SS_NO_WIDTH             SS_WIDTH(0)
#define SS_HEIGHT(_len)         "height:" #_len ";"
#define SS_NO_HEIGHT            SS_HEIGHT(0)
#define SS_BORDER(_len)         "border:" #_len ";"
#define SS_BORDER_RADIUS(_len)  "border-radius:" #_len ";"
#define SS_BORDER_WIDTH(_len)   "border-width:" #_len ";"
#define SS_BORDER_HEIGHT(_len)  "border-height:" #_len ";"
#define SS_BORDERLESS           "border-top:0;"
#define SS_MARGIN(_len)         "margin:" #_len ";"
#define SS_NO_MARGIN            SS_MARGIN(0)
#define SS_BORDER_IMAGE(_img)   "border-image:" #_img ";"
#define SS_BORDER_IMAGE_URL(_url) "border-image: url(" _url ");"
#define SS_SELECTION_COLOR(_color)"selection-color:" #_color ";"

#endif // _CORE_SS_H
