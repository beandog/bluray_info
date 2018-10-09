#include "bluray_pgs.h"

void bluray_pgs_lang(char *str, const uint8_t lang[4]) {

	memcpy(str, lang, BLURAY_PGS_LANG + 1);

}

void bluray_pgs_code(char *str, const uint8_t code) {

	switch(code) {

		case BLURAY_TEXT_CHAR_CODE_UTF8:
			strncpy(str, "utf8", BLURAY_PGS_CHAR_CODE + 1);
			break;

		case BLURAY_TEXT_CHAR_CODE_UTF16BE:
			strncpy(str, "utf16be", BLURAY_PGS_CHAR_CODE + 1);
			break;

		case BLURAY_TEXT_CHAR_CODE_SHIFT_JIS:
			strncpy(str, "shift_jis", BLURAY_PGS_CHAR_CODE + 1);
			break;

		case BLURAY_TEXT_CHAR_CODE_EUC_KR:
			strncpy(str, "euc_kr", BLURAY_PGS_CHAR_CODE + 1);
			break;

		case BLURAY_TEXT_CHAR_CODE_GB18030_20001:
			strncpy(str, "gb18030_20001", BLURAY_PGS_CHAR_CODE + 1);
			break;

		case BLURAY_TEXT_CHAR_CODE_CN_GB:
			strncpy(str, "cn_gb", BLURAY_PGS_CHAR_CODE + 1);
			break;

		case BLURAY_TEXT_CHAR_CODE_BIG5:
			strncpy(str, "big5", BLURAY_PGS_CHAR_CODE + 1);
			break;

		default:
			strncpy(str, "", BLURAY_PGS_CHAR_CODE + 1);
			break;

	}

}
