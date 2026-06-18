/* ANSI-C code produced by gperf version 3.0.1 */
/* Command-line: gperf -m 10 lib/aliases.gperf  */
/* Computed positions: -k'1,3-11,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "lib/aliases.gperf"
struct alias { int name; unsigned int encoding_index; };

#define TOTAL_KEYWORDS 358
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 45
#define MIN_HASH_VALUE 12
#define MAX_HASH_VALUE 1111
/* maximum key range = 1100, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
aliases_hash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112,   14,   54, 1112,   17,    4,
        11,    8,   65,    6,    5,  166,    9,   24,  247, 1112,
      1112, 1112, 1112, 1112, 1112,   41,  272,    4,   89,  142,
        80,   37,   42,    6,   12,  242,   13,  144,    6,    4,
       124, 1112,   89,   74,   13,  108,  213,   27,  314,   12,
         4, 1112, 1112, 1112, 1112,   33, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112,
      1112, 1112, 1112, 1112, 1112, 1112, 1112, 1112
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[10]];
      /*FALLTHROUGH*/
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str12[sizeof("CN")];
    char stringpool_str19[sizeof("L1")];
    char stringpool_str20[sizeof("L6")];
    char stringpool_str21[sizeof("L5")];
    char stringpool_str22[sizeof("866")];
    char stringpool_str23[sizeof("L3")];
    char stringpool_str24[sizeof("L8")];
    char stringpool_str26[sizeof("L2")];
    char stringpool_str33[sizeof("CP866")];
    char stringpool_str34[sizeof("862")];
    char stringpool_str35[sizeof("CP1361")];
    char stringpool_str39[sizeof("CP1251")];
    char stringpool_str41[sizeof("CP1256")];
    char stringpool_str42[sizeof("CP1133")];
    char stringpool_str43[sizeof("CP1255")];
    char stringpool_str45[sizeof("CP862")];
    char stringpool_str46[sizeof("850")];
    char stringpool_str47[sizeof("CP1253")];
    char stringpool_str48[sizeof("HZ")];
    char stringpool_str49[sizeof("CP1258")];
    char stringpool_str50[sizeof("L10")];
    char stringpool_str51[sizeof("CP936")];
    char stringpool_str52[sizeof("LATIN1")];
    char stringpool_str53[sizeof("CP1252")];
    char stringpool_str54[sizeof("LATIN6")];
    char stringpool_str55[sizeof("C99")];
    char stringpool_str56[sizeof("LATIN5")];
    char stringpool_str58[sizeof("CP850")];
    char stringpool_str60[sizeof("LATIN3")];
    char stringpool_str62[sizeof("LATIN8")];
    char stringpool_str63[sizeof("CP932")];
    char stringpool_str64[sizeof("CP50221")];
    char stringpool_str65[sizeof("CP1250")];
    char stringpool_str66[sizeof("LATIN2")];
    char stringpool_str68[sizeof("ASCII")];
    char stringpool_str70[sizeof("CP819")];
    char stringpool_str73[sizeof("CP950")];
    char stringpool_str75[sizeof("CP51932")];
    char stringpool_str80[sizeof("L4")];
    char stringpool_str83[sizeof("LATIN10")];
    char stringpool_str88[sizeof("GB2312")];
    char stringpool_str89[sizeof("ISO8859-1")];
    char stringpool_str91[sizeof("ISO8859-6")];
    char stringpool_str93[sizeof("ISO8859-5")];
    char stringpool_str94[sizeof("ISO8859-11")];
    char stringpool_str96[sizeof("ISO8859-16")];
    char stringpool_str97[sizeof("ISO8859-3")];
    char stringpool_str98[sizeof("ISO8859-15")];
    char stringpool_str99[sizeof("ISO8859-8")];
    char stringpool_str100[sizeof("R8")];
    char stringpool_str102[sizeof("ISO8859-13")];
    char stringpool_str103[sizeof("ISO8859-2")];
    char stringpool_str104[sizeof("ISO-8859-1")];
    char stringpool_str106[sizeof("ISO-8859-6")];
    char stringpool_str107[sizeof("LATIN-9")];
    char stringpool_str108[sizeof("ISO-8859-5")];
    char stringpool_str109[sizeof("ISO-8859-11")];
    char stringpool_str111[sizeof("ISO-8859-16")];
    char stringpool_str112[sizeof("ISO-8859-3")];
    char stringpool_str113[sizeof("ISO-8859-15")];
    char stringpool_str114[sizeof("ISO-8859-8")];
    char stringpool_str115[sizeof("ISO-2022-CN")];
    char stringpool_str116[sizeof("GB18030")];
    char stringpool_str117[sizeof("ISO-8859-13")];
    char stringpool_str118[sizeof("ISO-8859-2")];
    char stringpool_str119[sizeof("UHC")];
    char stringpool_str120[sizeof("ISO8859-10")];
    char stringpool_str123[sizeof("ISO_8859-1")];
    char stringpool_str124[sizeof("ISO646-CN")];
    char stringpool_str125[sizeof("ISO_8859-6")];
    char stringpool_str126[sizeof("ISO-2022-CN-EXT")];
    char stringpool_str127[sizeof("ISO_8859-5")];
    char stringpool_str128[sizeof("ISO_8859-11")];
    char stringpool_str129[sizeof("ISO8859-9")];
    char stringpool_str130[sizeof("ISO_8859-16")];
    char stringpool_str131[sizeof("ISO_8859-3")];
    char stringpool_str132[sizeof("ISO_8859-15")];
    char stringpool_str133[sizeof("ISO_8859-8")];
    char stringpool_str134[sizeof("ISO_8859-16:2001")];
    char stringpool_str135[sizeof("ISO-8859-10")];
    char stringpool_str136[sizeof("ISO_8859-13")];
    char stringpool_str137[sizeof("ISO_8859-2")];
    char stringpool_str138[sizeof("JP")];
    char stringpool_str140[sizeof("ISO_8859-15:1998")];
    char stringpool_str143[sizeof("TIS620")];
    char stringpool_str144[sizeof("ISO-8859-9")];
    char stringpool_str146[sizeof("CP949")];
    char stringpool_str147[sizeof("CYRILLIC")];
    char stringpool_str148[sizeof("CSASCII")];
    char stringpool_str149[sizeof("CP154")];
    char stringpool_str151[sizeof("ISO-IR-6")];
    char stringpool_str153[sizeof("ISO_8859-10:1992")];
    char stringpool_str154[sizeof("ISO_8859-10")];
    char stringpool_str155[sizeof("MAC")];
    char stringpool_str156[sizeof("JIS0208")];
    char stringpool_str158[sizeof("TIS-620")];
    char stringpool_str161[sizeof("CP1254")];
    char stringpool_str162[sizeof("ISO-IR-166")];
    char stringpool_str163[sizeof("ISO_8859-9")];
    char stringpool_str164[sizeof("ISO-IR-165")];
    char stringpool_str165[sizeof("CSISO2022CN")];
    char stringpool_str166[sizeof("ISO-IR-58")];
    char stringpool_str167[sizeof("EUCCN")];
    char stringpool_str168[sizeof("ISO-IR-126")];
    char stringpool_str171[sizeof("GB_2312-80")];
    char stringpool_str172[sizeof("ISO-IR-101")];
    char stringpool_str173[sizeof("ISO-IR-138")];
    char stringpool_str174[sizeof("LATIN4")];
    char stringpool_str175[sizeof("ISO-IR-226")];
    char stringpool_str176[sizeof("TIS620-0")];
    char stringpool_str180[sizeof("IBM866")];
    char stringpool_str181[sizeof("L7")];
    char stringpool_str182[sizeof("EUC-CN")];
    char stringpool_str183[sizeof("GB_1988-80")];
    char stringpool_str184[sizeof("US")];
    char stringpool_str185[sizeof("ISO-IR-110")];
    char stringpool_str186[sizeof("CSISOLATIN1")];
    char stringpool_str187[sizeof("ISO-IR-203")];
    char stringpool_str188[sizeof("CSISOLATIN6")];
    char stringpool_str190[sizeof("CSISOLATIN5")];
    char stringpool_str191[sizeof("MS936")];
    char stringpool_str192[sizeof("IBM862")];
    char stringpool_str193[sizeof("CSISOLATINCYRILLIC")];
    char stringpool_str194[sizeof("CSISOLATIN3")];
    char stringpool_str195[sizeof("JIS_C6226-1983")];
    char stringpool_str198[sizeof("ISO-IR-100")];
    char stringpool_str199[sizeof("ISO_8859-14:1998")];
    char stringpool_str200[sizeof("CSISOLATIN2")];
    char stringpool_str201[sizeof("ISO-IR-159")];
    char stringpool_str203[sizeof("MS932")];
    char stringpool_str204[sizeof("MS50221")];
    char stringpool_str205[sizeof("IBM850")];
    char stringpool_str206[sizeof("JIS_C6220-1969-RO")];
    char stringpool_str211[sizeof("ISO8859-4")];
    char stringpool_str212[sizeof("ISO-IR-109")];
    char stringpool_str215[sizeof("MS51932")];
    char stringpool_str216[sizeof("ISO8859-14")];
    char stringpool_str217[sizeof("IBM819")];
    char stringpool_str218[sizeof("EUCTW")];
    char stringpool_str219[sizeof("ISO-IR-199")];
    char stringpool_str220[sizeof("ISO-CELTIC")];
    char stringpool_str221[sizeof("SJIS-WIN")];
    char stringpool_str222[sizeof("UTF-16")];
    char stringpool_str223[sizeof("UCS-2")];
    char stringpool_str224[sizeof("TIS620.2533-1")];
    char stringpool_str225[sizeof("UTF-8")];
    char stringpool_str226[sizeof("ISO-8859-4")];
    char stringpool_str227[sizeof("CHAR")];
    char stringpool_str228[sizeof("CSISOLATINARABIC")];
    char stringpool_str230[sizeof("ISO-IR-148")];
    char stringpool_str231[sizeof("ISO-8859-14")];
    char stringpool_str232[sizeof("SJIS")];
    char stringpool_str233[sizeof("EUC-TW")];
    char stringpool_str237[sizeof("TIS620.2533-0")];
    char stringpool_str238[sizeof("UTF-32")];
    char stringpool_str241[sizeof("ISO-2022-JP-1")];
    char stringpool_str242[sizeof("TCVN")];
    char stringpool_str243[sizeof("TIS620.2529-1")];
    char stringpool_str245[sizeof("ISO_8859-4")];
    char stringpool_str248[sizeof("ISO-2022-JP-2")];
    char stringpool_str250[sizeof("ISO_8859-14")];
    char stringpool_str251[sizeof("CSISO159JISX02121990")];
    char stringpool_str252[sizeof("CSISOLATINHEBREW")];
    char stringpool_str253[sizeof("ELOT_928")];
    char stringpool_str260[sizeof("ISO-IR-149")];
    char stringpool_str263[sizeof("MACTHAI")];
    char stringpool_str265[sizeof("WCHAR_T")];
    char stringpool_str267[sizeof("US-ASCII")];
    char stringpool_str268[sizeof("ISO-10646-UCS-2")];
    char stringpool_str269[sizeof("PT154")];
    char stringpool_str273[sizeof("CSWINDOWS31J")];
    char stringpool_str274[sizeof("CSISO14JISC6220RO")];
    char stringpool_str275[sizeof("CSPC862LATINHEBREW")];
    char stringpool_str276[sizeof("ISO-IR-14")];
    char stringpool_str278[sizeof("WINDOWS-1251")];
    char stringpool_str279[sizeof("WINDOWS-1256")];
    char stringpool_str280[sizeof("WINDOWS-1255")];
    char stringpool_str282[sizeof("WINDOWS-1253")];
    char stringpool_str283[sizeof("WINDOWS-1258")];
    char stringpool_str284[sizeof("CYRILLIC-ASIAN")];
    char stringpool_str285[sizeof("WINDOWS-1252")];
    char stringpool_str288[sizeof("WINDOWS-31J")];
    char stringpool_str291[sizeof("WINDOWS-1250")];
    char stringpool_str292[sizeof("WINDOWS-50221")];
    char stringpool_str294[sizeof("WINDOWS-936")];
    char stringpool_str296[sizeof("MS-CYRL")];
    char stringpool_str297[sizeof("CSISO2022JP2")];
    char stringpool_str298[sizeof("MS-ANSI")];
    char stringpool_str299[sizeof("WINDOWS-51932")];
    char stringpool_str303[sizeof("KOI8-T")];
    char stringpool_str304[sizeof("ROMAN8")];
    char stringpool_str306[sizeof("WINDOWS-932")];
    char stringpool_str307[sizeof("GEORGIAN-ACADEMY")];
    char stringpool_str308[sizeof("CSISOLATIN4")];
    char stringpool_str310[sizeof("MACCYRILLIC")];
    char stringpool_str311[sizeof("JAVA")];
    char stringpool_str312[sizeof("ISO-2022-JP-MS")];
    char stringpool_str314[sizeof("CP874")];
    char stringpool_str315[sizeof("VISCII")];
    char stringpool_str316[sizeof("ARMSCII-8")];
    char stringpool_str322[sizeof("ISO-10646-UCS-4")];
    char stringpool_str323[sizeof("KSC_5601")];
    char stringpool_str325[sizeof("BIG5")];
    char stringpool_str326[sizeof("CSUCS4")];
    char stringpool_str327[sizeof("CSVISCII")];
    char stringpool_str331[sizeof("UCS-4")];
    char stringpool_str332[sizeof("CSEUCTW")];
    char stringpool_str334[sizeof("IBM-CP1133")];
    char stringpool_str337[sizeof("CSBIG5")];
    char stringpool_str339[sizeof("WINDOWS-1254")];
    char stringpool_str340[sizeof("BIG-5")];
    char stringpool_str342[sizeof("ISO-IR-144")];
    char stringpool_str344[sizeof("MACINTOSH")];
    char stringpool_str350[sizeof("EUCJP-WIN")];
    char stringpool_str352[sizeof("CN-BIG5")];
    char stringpool_str354[sizeof("CP367")];
    char stringpool_str355[sizeof("X0201")];
    char stringpool_str356[sizeof("X0212")];
    char stringpool_str359[sizeof("ISO-2022-JP")];
    char stringpool_str361[sizeof("ISO-IR-179")];
    char stringpool_str362[sizeof("SHIFT-JIS")];
    char stringpool_str363[sizeof("CP1257")];
    char stringpool_str364[sizeof("ISO646-US")];
    char stringpool_str365[sizeof("X0208")];
    char stringpool_str366[sizeof("CSGB2312")];
    char stringpool_str367[sizeof("HP-ROMAN8")];
    char stringpool_str368[sizeof("ISO646-JP")];
    char stringpool_str369[sizeof("MACCROATIAN")];
    char stringpool_str374[sizeof("ARABIC")];
    char stringpool_str375[sizeof("ECMA-118")];
    char stringpool_str376[sizeof("LATIN7")];
    char stringpool_str381[sizeof("SHIFT_JIS")];
    char stringpool_str382[sizeof("ISO_8859-5:1988")];
    char stringpool_str384[sizeof("ISO_8859-3:1988")];
    char stringpool_str385[sizeof("ISO_8859-8:1988")];
    char stringpool_str386[sizeof("CSUNICODE11")];
    char stringpool_str388[sizeof("MULELAO-1")];
    char stringpool_str391[sizeof("CSMACINTOSH")];
    char stringpool_str395[sizeof("CSSHIFTJIS")];
    char stringpool_str398[sizeof("VISCII1.1-1")];
    char stringpool_str399[sizeof("PTCP154")];
    char stringpool_str400[sizeof("KS_C_5601-1989")];
    char stringpool_str404[sizeof("UNICODE-1-1")];
    char stringpool_str409[sizeof("CSISO2022JP")];
    char stringpool_str411[sizeof("EUCJP")];
    char stringpool_str412[sizeof("ASMO-708")];
    char stringpool_str413[sizeof("ISO8859-7")];
    char stringpool_str415[sizeof("ISO_8859-9:1989")];
    char stringpool_str417[sizeof("CSHALFWIDTHKATAKANA")];
    char stringpool_str418[sizeof("CSPTCP154")];
    char stringpool_str426[sizeof("EUC-JP")];
    char stringpool_str428[sizeof("ISO-8859-7")];
    char stringpool_str434[sizeof("HZ-GB-2312")];
    char stringpool_str440[sizeof("WINDOWS-1257")];
    char stringpool_str441[sizeof("ISO_8859-4:1988")];
    char stringpool_str446[sizeof("MACROMAN")];
    char stringpool_str447[sizeof("ISO_8859-7")];
    char stringpool_str450[sizeof("CSKOI8R")];
    char stringpool_str451[sizeof("TCVN5712-1")];
    char stringpool_str453[sizeof("TCVN-5712")];
    char stringpool_str455[sizeof("KOI8-R")];
    char stringpool_str458[sizeof("CSIBM866")];
    char stringpool_str459[sizeof("SJIS-OPEN")];
    char stringpool_str461[sizeof("CSISOLATINGREEK")];
    char stringpool_str463[sizeof("CSISO58GB231280")];
    char stringpool_str467[sizeof("SJIS-MS")];
    char stringpool_str474[sizeof("CSPC850MULTILINGUAL")];
    char stringpool_str479[sizeof("SHIFFT_JIS-MS")];
    char stringpool_str480[sizeof("ISO-IR-57")];
    char stringpool_str482[sizeof("CSHPROMAN8")];
    char stringpool_str483[sizeof("ISO-IR-87")];
    char stringpool_str485[sizeof("ISO-IR-157")];
    char stringpool_str487[sizeof("ECMA-114")];
    char stringpool_str490[sizeof("ISO-IR-127")];
    char stringpool_str493[sizeof("KOI8-U")];
    char stringpool_str495[sizeof("JIS_X0201")];
    char stringpool_str496[sizeof("JIS_X0212")];
    char stringpool_str498[sizeof("MS_KANJI")];
    char stringpool_str500[sizeof("ISO_646.IRV:1991")];
    char stringpool_str501[sizeof("IBM367")];
    char stringpool_str504[sizeof("UCS-2-INTERNAL")];
    char stringpool_str505[sizeof("JIS_X0208")];
    char stringpool_str509[sizeof("JISX0201-1976")];
    char stringpool_str511[sizeof("UCS-2LE")];
    char stringpool_str514[sizeof("CSUNICODE")];
    char stringpool_str516[sizeof("UTF-16LE")];
    char stringpool_str517[sizeof("GEORGIAN-PS")];
    char stringpool_str519[sizeof("ISO-2022-KR")];
    char stringpool_str523[sizeof("CHINESE")];
    char stringpool_str524[sizeof("GBK")];
    char stringpool_str525[sizeof("JIS_X0212-1990")];
    char stringpool_str526[sizeof("UTF-32LE")];
    char stringpool_str527[sizeof("JIS_X0208-1983")];
    char stringpool_str530[sizeof("MACROMANIA")];
    char stringpool_str532[sizeof("KOREAN")];
    char stringpool_str536[sizeof("JIS_X0208-1990")];
    char stringpool_str537[sizeof("ISO_8859-1:1987")];
    char stringpool_str538[sizeof("ISO_8859-6:1987")];
    char stringpool_str539[sizeof("UTF-7")];
    char stringpool_str540[sizeof("CN-GB-ISOIR165")];
    char stringpool_str541[sizeof("ISO_8859-7:2003")];
    char stringpool_str542[sizeof("KS_C_5601-1987")];
    char stringpool_str544[sizeof("ISO_8859-2:1987")];
    char stringpool_str548[sizeof("MACICELAND")];
    char stringpool_str552[sizeof("CSUNICODE11UTF7")];
    char stringpool_str553[sizeof("UNICODELITTLE")];
    char stringpool_str557[sizeof("WINDOWS-874")];
    char stringpool_str558[sizeof("UCS-4-INTERNAL")];
    char stringpool_str563[sizeof("CSKSC56011987")];
    char stringpool_str565[sizeof("UCS-4LE")];
    char stringpool_str567[sizeof("JIS_X0212.1990-0")];
    char stringpool_str569[sizeof("CSISO2022KR")];
    char stringpool_str571[sizeof("EUCKR")];
    char stringpool_str572[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str581[sizeof("EUCJPMS")];
    char stringpool_str583[sizeof("KOI8-RU")];
    char stringpool_str586[sizeof("EUC-KR")];
    char stringpool_str587[sizeof("GREEK8")];
    char stringpool_str588[sizeof("EUCJP-OPEN")];
    char stringpool_str589[sizeof("MS-EE")];
    char stringpool_str596[sizeof("EUCJP-MS")];
    char stringpool_str604[sizeof("CN-GB")];
    char stringpool_str605[sizeof("HEBREW")];
    char stringpool_str611[sizeof("EUC-JP-MS")];
    char stringpool_str614[sizeof("MACARABIC")];
    char stringpool_str619[sizeof("CSISO57GB1988")];
    char stringpool_str632[sizeof("ANSI_X3.4-1986")];
    char stringpool_str636[sizeof("ANSI_X3.4-1968")];
    char stringpool_str644[sizeof("JOHAB")];
    char stringpool_str685[sizeof("CSEUCKR")];
    char stringpool_str693[sizeof("CSISO87JISX0208")];
    char stringpool_str699[sizeof("ISO_8859-7:1987")];
    char stringpool_str707[sizeof("TCVN5712-1:1993")];
    char stringpool_str713[sizeof("UCS-2-SWAPPED")];
    char stringpool_str715[sizeof("UNICODEBIG")];
    char stringpool_str756[sizeof("MACCENTRALEUROPE")];
    char stringpool_str765[sizeof("WINBALTRIM")];
    char stringpool_str767[sizeof("UCS-4-SWAPPED")];
    char stringpool_str770[sizeof("UCS-2BE")];
    char stringpool_str774[sizeof("MACTURKISH")];
    char stringpool_str775[sizeof("UTF-16BE")];
    char stringpool_str785[sizeof("UTF-32BE")];
    char stringpool_str799[sizeof("MS-HEBR")];
    char stringpool_str810[sizeof("GREEK")];
    char stringpool_str818[sizeof("NEXTSTEP")];
    char stringpool_str824[sizeof("UCS-4BE")];
    char stringpool_str834[sizeof("BIG5HKSCS")];
    char stringpool_str849[sizeof("BIG5-HKSCS")];
    char stringpool_str859[sizeof("MS-TURK")];
    char stringpool_str880[sizeof("MS-ARAB")];
    char stringpool_str898[sizeof("MACHEBREW")];
    char stringpool_str899[sizeof("BIGFIVE")];
    char stringpool_str914[sizeof("BIG-FIVE")];
    char stringpool_str934[sizeof("MACUKRAINE")];
    char stringpool_str957[sizeof("EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE")];
    char stringpool_str1050[sizeof("MACGREEK")];
    char stringpool_str1060[sizeof("MS-GREEK")];
    char stringpool_str1111[sizeof("CSEUCPKDFMTJAPANESE")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "CN",
    "L1",
    "L6",
    "L5",
    "866",
    "L3",
    "L8",
    "L2",
    "CP866",
    "862",
    "CP1361",
    "CP1251",
    "CP1256",
    "CP1133",
    "CP1255",
    "CP862",
    "850",
    "CP1253",
    "HZ",
    "CP1258",
    "L10",
    "CP936",
    "LATIN1",
    "CP1252",
    "LATIN6",
    "C99",
    "LATIN5",
    "CP850",
    "LATIN3",
    "LATIN8",
    "CP932",
    "CP50221",
    "CP1250",
    "LATIN2",
    "ASCII",
    "CP819",
    "CP950",
    "CP51932",
    "L4",
    "LATIN10",
    "GB2312",
    "ISO8859-1",
    "ISO8859-6",
    "ISO8859-5",
    "ISO8859-11",
    "ISO8859-16",
    "ISO8859-3",
    "ISO8859-15",
    "ISO8859-8",
    "R8",
    "ISO8859-13",
    "ISO8859-2",
    "ISO-8859-1",
    "ISO-8859-6",
    "LATIN-9",
    "ISO-8859-5",
    "ISO-8859-11",
    "ISO-8859-16",
    "ISO-8859-3",
    "ISO-8859-15",
    "ISO-8859-8",
    "ISO-2022-CN",
    "GB18030",
    "ISO-8859-13",
    "ISO-8859-2",
    "UHC",
    "ISO8859-10",
    "ISO_8859-1",
    "ISO646-CN",
    "ISO_8859-6",
    "ISO-2022-CN-EXT",
    "ISO_8859-5",
    "ISO_8859-11",
    "ISO8859-9",
    "ISO_8859-16",
    "ISO_8859-3",
    "ISO_8859-15",
    "ISO_8859-8",
    "ISO_8859-16:2001",
    "ISO-8859-10",
    "ISO_8859-13",
    "ISO_8859-2",
    "JP",
    "ISO_8859-15:1998",
    "TIS620",
    "ISO-8859-9",
    "CP949",
    "CYRILLIC",
    "CSASCII",
    "CP154",
    "ISO-IR-6",
    "ISO_8859-10:1992",
    "ISO_8859-10",
    "MAC",
    "JIS0208",
    "TIS-620",
    "CP1254",
    "ISO-IR-166",
    "ISO_8859-9",
    "ISO-IR-165",
    "CSISO2022CN",
    "ISO-IR-58",
    "EUCCN",
    "ISO-IR-126",
    "GB_2312-80",
    "ISO-IR-101",
    "ISO-IR-138",
    "LATIN4",
    "ISO-IR-226",
    "TIS620-0",
    "IBM866",
    "L7",
    "EUC-CN",
    "GB_1988-80",
    "US",
    "ISO-IR-110",
    "CSISOLATIN1",
    "ISO-IR-203",
    "CSISOLATIN6",
    "CSISOLATIN5",
    "MS936",
    "IBM862",
    "CSISOLATINCYRILLIC",
    "CSISOLATIN3",
    "JIS_C6226-1983",
    "ISO-IR-100",
    "ISO_8859-14:1998",
    "CSISOLATIN2",
    "ISO-IR-159",
    "MS932",
    "MS50221",
    "IBM850",
    "JIS_C6220-1969-RO",
    "ISO8859-4",
    "ISO-IR-109",
    "MS51932",
    "ISO8859-14",
    "IBM819",
    "EUCTW",
    "ISO-IR-199",
    "ISO-CELTIC",
    "SJIS-WIN",
    "UTF-16",
    "UCS-2",
    "TIS620.2533-1",
    "UTF-8",
    "ISO-8859-4",
    "CHAR",
    "CSISOLATINARABIC",
    "ISO-IR-148",
    "ISO-8859-14",
    "SJIS",
    "EUC-TW",
    "TIS620.2533-0",
    "UTF-32",
    "ISO-2022-JP-1",
    "TCVN",
    "TIS620.2529-1",
    "ISO_8859-4",
    "ISO-2022-JP-2",
    "ISO_8859-14",
    "CSISO159JISX02121990",
    "CSISOLATINHEBREW",
    "ELOT_928",
    "ISO-IR-149",
    "MACTHAI",
    "WCHAR_T",
    "US-ASCII",
    "ISO-10646-UCS-2",
    "PT154",
    "CSWINDOWS31J",
    "CSISO14JISC6220RO",
    "CSPC862LATINHEBREW",
    "ISO-IR-14",
    "WINDOWS-1251",
    "WINDOWS-1256",
    "WINDOWS-1255",
    "WINDOWS-1253",
    "WINDOWS-1258",
    "CYRILLIC-ASIAN",
    "WINDOWS-1252",
    "WINDOWS-31J",
    "WINDOWS-1250",
    "WINDOWS-50221",
    "WINDOWS-936",
    "MS-CYRL",
    "CSISO2022JP2",
    "MS-ANSI",
    "WINDOWS-51932",
    "KOI8-T",
    "ROMAN8",
    "WINDOWS-932",
    "GEORGIAN-ACADEMY",
    "CSISOLATIN4",
    "MACCYRILLIC",
    "JAVA",
    "ISO-2022-JP-MS",
    "CP874",
    "VISCII",
    "ARMSCII-8",
    "ISO-10646-UCS-4",
    "KSC_5601",
    "BIG5",
    "CSUCS4",
    "CSVISCII",
    "UCS-4",
    "CSEUCTW",
    "IBM-CP1133",
    "CSBIG5",
    "WINDOWS-1254",
    "BIG-5",
    "ISO-IR-144",
    "MACINTOSH",
    "EUCJP-WIN",
    "CN-BIG5",
    "CP367",
    "X0201",
    "X0212",
    "ISO-2022-JP",
    "ISO-IR-179",
    "SHIFT-JIS",
    "CP1257",
    "ISO646-US",
    "X0208",
    "CSGB2312",
    "HP-ROMAN8",
    "ISO646-JP",
    "MACCROATIAN",
    "ARABIC",
    "ECMA-118",
    "LATIN7",
    "SHIFT_JIS",
    "ISO_8859-5:1988",
    "ISO_8859-3:1988",
    "ISO_8859-8:1988",
    "CSUNICODE11",
    "MULELAO-1",
    "CSMACINTOSH",
    "CSSHIFTJIS",
    "VISCII1.1-1",
    "PTCP154",
    "KS_C_5601-1989",
    "UNICODE-1-1",
    "CSISO2022JP",
    "EUCJP",
    "ASMO-708",
    "ISO8859-7",
    "ISO_8859-9:1989",
    "CSHALFWIDTHKATAKANA",
    "CSPTCP154",
    "EUC-JP",
    "ISO-8859-7",
    "HZ-GB-2312",
    "WINDOWS-1257",
    "ISO_8859-4:1988",
    "MACROMAN",
    "ISO_8859-7",
    "CSKOI8R",
    "TCVN5712-1",
    "TCVN-5712",
    "KOI8-R",
    "CSIBM866",
    "SJIS-OPEN",
    "CSISOLATINGREEK",
    "CSISO58GB231280",
    "SJIS-MS",
    "CSPC850MULTILINGUAL",
    "SHIFFT_JIS-MS",
    "ISO-IR-57",
    "CSHPROMAN8",
    "ISO-IR-87",
    "ISO-IR-157",
    "ECMA-114",
    "ISO-IR-127",
    "KOI8-U",
    "JIS_X0201",
    "JIS_X0212",
    "MS_KANJI",
    "ISO_646.IRV:1991",
    "IBM367",
    "UCS-2-INTERNAL",
    "JIS_X0208",
    "JISX0201-1976",
    "UCS-2LE",
    "CSUNICODE",
    "UTF-16LE",
    "GEORGIAN-PS",
    "ISO-2022-KR",
    "CHINESE",
    "GBK",
    "JIS_X0212-1990",
    "UTF-32LE",
    "JIS_X0208-1983",
    "MACROMANIA",
    "KOREAN",
    "JIS_X0208-1990",
    "ISO_8859-1:1987",
    "ISO_8859-6:1987",
    "UTF-7",
    "CN-GB-ISOIR165",
    "ISO_8859-7:2003",
    "KS_C_5601-1987",
    "ISO_8859-2:1987",
    "MACICELAND",
    "CSUNICODE11UTF7",
    "UNICODELITTLE",
    "WINDOWS-874",
    "UCS-4-INTERNAL",
    "CSKSC56011987",
    "UCS-4LE",
    "JIS_X0212.1990-0",
    "CSISO2022KR",
    "EUCKR",
    "UNICODE-1-1-UTF-7",
    "EUCJPMS",
    "KOI8-RU",
    "EUC-KR",
    "GREEK8",
    "EUCJP-OPEN",
    "MS-EE",
    "EUCJP-MS",
    "CN-GB",
    "HEBREW",
    "EUC-JP-MS",
    "MACARABIC",
    "CSISO57GB1988",
    "ANSI_X3.4-1986",
    "ANSI_X3.4-1968",
    "JOHAB",
    "CSEUCKR",
    "CSISO87JISX0208",
    "ISO_8859-7:1987",
    "TCVN5712-1:1993",
    "UCS-2-SWAPPED",
    "UNICODEBIG",
    "MACCENTRALEUROPE",
    "WINBALTRIM",
    "UCS-4-SWAPPED",
    "UCS-2BE",
    "MACTURKISH",
    "UTF-16BE",
    "UTF-32BE",
    "MS-HEBR",
    "GREEK",
    "NEXTSTEP",
    "UCS-4BE",
    "BIG5HKSCS",
    "BIG5-HKSCS",
    "MS-TURK",
    "MS-ARAB",
    "MACHEBREW",
    "BIGFIVE",
    "BIG-FIVE",
    "MACUKRAINE",
    "EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE",
    "MACGREEK",
    "MS-GREEK",
    "CSEUCPKDFMTJAPANESE"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 283 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str12, ei_iso646_cn},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 60 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str19, ei_iso8859_1},
#line 134 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str20, ei_iso8859_10},
#line 126 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str21, ei_iso8859_9},
#line 207 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str22, ei_cp866},
#line 76 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str23, ei_iso8859_3},
#line 151 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str24, ei_iso8859_14},
    {-1},
#line 68 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str26, ei_iso8859_2},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 205 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str33, ei_cp866},
#line 203 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str34, ei_cp862},
#line 365 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str35, ei_johab},
    {-1}, {-1}, {-1},
#line 174 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str39, ei_cp1251},
    {-1},
#line 189 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str41, ei_cp1256},
#line 239 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str42, ei_cp1133},
#line 186 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str43, ei_cp1255},
    {-1},
#line 201 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str45, ei_cp862},
#line 199 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str46, ei_cp850},
#line 180 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str47, ei_cp1253},
#line 345 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str48, ei_hz},
#line 195 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str49, ei_cp1258},
#line 165 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str50, ei_iso8859_16},
#line 338 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str51, ei_ces_gbk},
#line 59 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str52, ei_iso8859_1},
#line 177 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str53, ei_cp1252},
#line 133 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str54, ei_iso8859_10},
#line 51 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str55, ei_c99},
#line 125 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str56, ei_iso8859_9},
    {-1},
#line 197 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str58, ei_cp850},
    {-1},
#line 75 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str60, ei_iso8859_3},
    {-1},
#line 150 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str62, ei_iso8859_14},
#line 314 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str63, ei_cp932},
#line 326 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str64, ei_iso2022_jpms},
#line 171 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str65, ei_cp1250},
#line 67 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str66, ei_iso8859_2},
    {-1},
#line 13 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str68, ei_ascii},
    {-1},
#line 57 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str70, ei_iso8859_1},
    {-1}, {-1},
#line 356 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str73, ei_cp950},
    {-1},
#line 306 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str75, ei_cp51932},
    {-1}, {-1}, {-1}, {-1},
#line 84 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str80, ei_iso8859_4},
    {-1}, {-1},
#line 164 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str83, ei_iso8859_16},
    {-1}, {-1}, {-1}, {-1},
#line 334 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str88, ei_euc_cn},
#line 62 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str89, ei_iso8859_1},
    {-1},
#line 102 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str91, ei_iso8859_6},
    {-1},
#line 93 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str93, ei_iso8859_5},
#line 139 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str94, ei_iso8859_11},
    {-1},
#line 166 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str96, ei_iso8859_16},
#line 78 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str97, ei_iso8859_3},
#line 159 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str98, ei_iso8859_15},
#line 120 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str99, ei_iso8859_8},
#line 226 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str100, ei_hp_roman8},
    {-1},
#line 145 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str102, ei_iso8859_13},
#line 70 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str103, ei_iso8859_2},
#line 53 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str104, ei_iso8859_1},
    {-1},
#line 94 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str106, ei_iso8859_6},
#line 158 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str107, ei_iso8859_15},
#line 87 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str108, ei_iso8859_5},
#line 137 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str109, ei_iso8859_11},
    {-1},
#line 160 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str111, ei_iso8859_16},
#line 71 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str112, ei_iso8859_3},
#line 154 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str113, ei_iso8859_15},
#line 114 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str114, ei_iso8859_8},
#line 342 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str115, ei_iso2022_cn},
#line 341 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str116, ei_gb18030},
#line 140 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str117, ei_iso8859_13},
#line 63 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str118, ei_iso8859_2},
#line 363 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str119, ei_cp949},
#line 136 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str120, ei_iso8859_10},
    {-1}, {-1},
#line 54 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str123, ei_iso8859_1},
#line 281 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str124, ei_iso646_cn},
#line 95 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str125, ei_iso8859_6},
#line 344 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str126, ei_iso2022_cn_ext},
#line 88 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str127, ei_iso8859_5},
#line 138 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str128, ei_iso8859_11},
#line 128 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str129, ei_iso8859_9},
#line 161 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str130, ei_iso8859_16},
#line 72 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str131, ei_iso8859_3},
#line 155 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str132, ei_iso8859_15},
#line 115 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str133, ei_iso8859_8},
#line 162 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str134, ei_iso8859_16},
#line 129 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str135, ei_iso8859_10},
#line 141 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str136, ei_iso8859_13},
#line 64 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str137, ei_iso8859_2},
#line 260 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str138, ei_iso646_jp},
    {-1},
#line 156 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str140, ei_iso8859_15},
    {-1}, {-1},
#line 242 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str143, ei_tis620},
#line 121 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str144, ei_iso8859_9},
    {-1},
#line 362 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str146, ei_cp949},
#line 91 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str147, ei_iso8859_5},
#line 22 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str148, ei_ascii},
#line 235 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str149, ei_pt154},
    {-1},
#line 16 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str151, ei_ascii},
    {-1},
#line 131 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str153, ei_iso8859_10},
#line 130 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str154, ei_iso8859_10},
#line 211 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str155, ei_mac_roman},
#line 269 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str156, ei_jisx0208},
    {-1},
#line 241 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str158, ei_tis620},
    {-1}, {-1},
#line 183 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str161, ei_cp1254},
#line 247 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str162, ei_tis620},
#line 122 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str163, ei_iso8859_9},
#line 289 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str164, ei_isoir165},
#line 343 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str165, ei_iso2022_cn},
#line 286 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str166, ei_gb2312},
#line 333 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str167, ei_euc_cn},
#line 107 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str168, ei_iso8859_7},
    {-1}, {-1},
#line 285 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str171, ei_gb2312},
#line 66 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str172, ei_iso8859_2},
#line 117 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str173, ei_iso8859_8},
#line 83 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str174, ei_iso8859_4},
#line 163 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str175, ei_iso8859_16},
#line 243 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str176, ei_tis620},
    {-1}, {-1}, {-1},
#line 206 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str180, ei_cp866},
#line 144 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str181, ei_iso8859_13},
#line 332 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str182, ei_euc_cn},
#line 280 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str183, ei_iso646_cn},
#line 21 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str184, ei_ascii},
#line 82 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str185, ei_iso8859_4},
#line 61 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str186, ei_iso8859_1},
#line 157 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str187, ei_iso8859_15},
#line 135 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str188, ei_iso8859_10},
    {-1},
#line 127 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str190, ei_iso8859_9},
#line 339 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str191, ei_ces_gbk},
#line 202 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str192, ei_cp862},
#line 92 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str193, ei_iso8859_5},
#line 77 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str194, ei_iso8859_3},
#line 272 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str195, ei_jisx0208},
    {-1}, {-1},
#line 56 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str198, ei_iso8859_1},
#line 148 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str199, ei_iso8859_14},
#line 69 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str200, ei_iso8859_2},
#line 278 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str201, ei_jisx0212},
    {-1},
#line 320 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str203, ei_cp932},
#line 328 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str204, ei_iso2022_jpms},
#line 198 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str205, ei_cp850},
#line 257 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str206, ei_iso646_jp},
    {-1}, {-1}, {-1}, {-1},
#line 86 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str211, ei_iso8859_4},
#line 74 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str212, ei_iso8859_3},
    {-1}, {-1},
#line 308 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str215, ei_cp51932},
#line 153 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str216, ei_iso8859_14},
#line 58 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str217, ei_iso8859_1},
#line 348 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str218, ei_euc_tw},
#line 149 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str219, ei_iso8859_14},
#line 152 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str220, ei_iso8859_14},
#line 318 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str221, ei_cp932},
#line 38 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str222, ei_utf16},
#line 24 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str223, ei_ucs2},
#line 246 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str224, ei_tis620},
#line 23 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str225, ei_utf8},
#line 79 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str226, ei_iso8859_4},
#line 368 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str227, ei_local_char},
#line 101 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str228, ei_iso8859_6},
    {-1},
#line 124 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str230, ei_iso8859_9},
#line 146 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str231, ei_iso8859_14},
#line 311 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str232, ei_sjis},
#line 347 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str233, ei_euc_tw},
    {-1}, {-1}, {-1},
#line 245 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str237, ei_tis620},
#line 41 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str238, ei_utf32},
    {-1}, {-1},
#line 329 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str241, ei_iso2022_jp1},
#line 253 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str242, ei_tcvn},
#line 244 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str243, ei_tis620},
    {-1},
#line 80 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str245, ei_iso8859_4},
    {-1}, {-1},
#line 330 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str248, ei_iso2022_jp2},
    {-1},
#line 147 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str250, ei_iso8859_14},
#line 279 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str251, ei_jisx0212},
#line 119 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str252, ei_iso8859_8},
#line 109 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str253, ei_iso8859_7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 294 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str260, ei_ksc5601},
    {-1}, {-1},
#line 223 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str263, ei_mac_thai},
    {-1},
#line 369 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str265, ei_local_wchar_t},
    {-1},
#line 12 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str267, ei_ascii},
#line 25 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str268, ei_ucs2},
#line 233 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str269, ei_pt154},
    {-1}, {-1}, {-1},
#line 316 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str273, ei_cp932},
#line 261 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str274, ei_iso646_jp},
#line 204 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str275, ei_cp862},
#line 259 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str276, ei_iso646_jp},
    {-1},
#line 175 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str278, ei_cp1251},
#line 190 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str279, ei_cp1256},
#line 187 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str280, ei_cp1255},
    {-1},
#line 181 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str282, ei_cp1253},
#line 196 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str283, ei_cp1258},
#line 236 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str284, ei_pt154},
#line 178 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str285, ei_cp1252},
    {-1}, {-1},
#line 315 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str288, ei_cp932},
    {-1}, {-1},
#line 172 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str291, ei_cp1250},
#line 327 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str292, ei_iso2022_jpms},
    {-1},
#line 340 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str294, ei_ces_gbk},
    {-1},
#line 176 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str296, ei_cp1251},
#line 331 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str297, ei_iso2022_jp2},
#line 179 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str298, ei_cp1252},
#line 307 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str299, ei_cp51932},
    {-1}, {-1}, {-1},
#line 232 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str303, ei_koi8_t},
#line 225 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str304, ei_hp_roman8},
    {-1},
#line 319 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str306, ei_cp932},
#line 230 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str307, ei_georgian_academy},
#line 85 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str308, ei_iso8859_4},
    {-1},
#line 217 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str310, ei_mac_cyrillic},
#line 52 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str311, ei_java},
#line 325 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str312, ei_iso2022_jpms},
    {-1},
#line 248 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str314, ei_cp874},
#line 250 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str315, ei_viscii},
#line 229 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str316, ei_armscii_8},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 34 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str322, ei_ucs4},
#line 291 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str323, ei_ksc5601},
    {-1},
#line 350 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str325, ei_ces_big5},
#line 35 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str326, ei_ucs4},
#line 252 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str327, ei_viscii},
    {-1}, {-1}, {-1},
#line 33 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str331, ei_ucs4},
#line 349 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str332, ei_euc_tw},
    {-1},
#line 240 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str334, ei_cp1133},
    {-1}, {-1},
#line 355 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str337, ei_ces_big5},
    {-1},
#line 184 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str339, ei_cp1254},
#line 351 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str340, ei_ces_big5},
    {-1},
#line 90 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str342, ei_iso8859_5},
    {-1},
#line 210 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str344, ei_mac_roman},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 304 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str350, ei_eucjp_ms},
    {-1},
#line 354 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str352, ei_ces_big5},
    {-1},
#line 19 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str354, ei_ascii},
#line 264 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str355, ei_jisx0201},
#line 277 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str356, ei_jisx0212},
    {-1}, {-1},
#line 323 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str359, ei_iso2022_jp},
    {-1},
#line 142 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str361, ei_iso8859_13},
#line 310 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str362, ei_sjis},
#line 192 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str363, ei_cp1257},
#line 14 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str364, ei_ascii},
#line 270 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str365, ei_jisx0208},
#line 336 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str366, ei_euc_cn},
#line 224 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str367, ei_hp_roman8},
#line 258 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str368, ei_iso646_jp},
#line 215 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str369, ei_mac_croatian},
    {-1}, {-1}, {-1}, {-1},
#line 100 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str374, ei_iso8859_6},
#line 108 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str375, ei_iso8859_7},
#line 143 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str376, ei_iso8859_13},
    {-1}, {-1}, {-1}, {-1},
#line 309 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str381, ei_sjis},
#line 89 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str382, ei_iso8859_5},
    {-1},
#line 73 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str384, ei_iso8859_3},
#line 116 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str385, ei_iso8859_8},
#line 30 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str386, ei_ucs2be},
    {-1},
#line 238 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str388, ei_mulelao},
    {-1}, {-1},
#line 212 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str391, ei_mac_roman},
    {-1}, {-1}, {-1},
#line 313 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str395, ei_sjis},
    {-1}, {-1},
#line 251 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str398, ei_viscii},
#line 234 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str399, ei_pt154},
#line 293 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str400, ei_ksc5601},
    {-1}, {-1}, {-1},
#line 29 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str404, ei_ucs2be},
    {-1}, {-1}, {-1}, {-1},
#line 324 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str409, ei_iso2022_jp},
    {-1},
#line 298 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str411, ei_euc_jp},
#line 99 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str412, ei_iso8859_6},
#line 113 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str413, ei_iso8859_7},
    {-1},
#line 123 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str415, ei_iso8859_9},
    {-1},
#line 265 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str417, ei_jisx0201},
#line 237 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str418, ei_pt154},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 297 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str426, ei_euc_jp},
    {-1},
#line 103 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str428, ei_iso8859_7},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 346 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str434, ei_hz},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 193 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str440, ei_cp1257},
#line 81 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str441, ei_iso8859_4},
    {-1}, {-1}, {-1}, {-1},
#line 209 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str446, ei_mac_roman},
#line 104 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str447, ei_iso8859_7},
    {-1}, {-1},
#line 168 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str450, ei_koi8_r},
#line 255 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str451, ei_tcvn},
    {-1},
#line 254 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str453, ei_tcvn},
    {-1},
#line 167 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str455, ei_koi8_r},
    {-1}, {-1},
#line 208 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str458, ei_cp866},
#line 317 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str459, ei_cp932},
    {-1},
#line 112 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str461, ei_iso8859_7},
    {-1},
#line 287 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str463, ei_gb2312},
    {-1}, {-1}, {-1},
#line 322 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str467, ei_cp932},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 200 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str474, ei_cp850},
    {-1}, {-1}, {-1}, {-1},
#line 321 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str479, ei_cp932},
#line 282 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str480, ei_iso646_cn},
    {-1},
#line 227 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str482, ei_hp_roman8},
#line 271 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str483, ei_jisx0208},
    {-1},
#line 132 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str485, ei_iso8859_10},
    {-1},
#line 98 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str487, ei_iso8859_6},
    {-1}, {-1},
#line 97 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str490, ei_iso8859_6},
    {-1}, {-1},
#line 169 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str493, ei_koi8_u},
    {-1},
#line 262 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str495, ei_jisx0201},
#line 274 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str496, ei_jisx0212},
    {-1},
#line 312 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str498, ei_sjis},
    {-1},
#line 15 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str500, ei_ascii},
#line 20 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str501, ei_ascii},
    {-1}, {-1},
#line 47 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str504, ei_ucs2internal},
#line 266 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str505, ei_jisx0208},
    {-1}, {-1}, {-1},
#line 263 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str509, ei_jisx0201},
    {-1},
#line 31 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str511, ei_ucs2le},
    {-1}, {-1},
#line 26 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str514, ei_ucs2},
    {-1},
#line 40 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str516, ei_utf16le},
#line 231 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str517, ei_georgian_ps},
    {-1},
#line 366 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str519, ei_iso2022_kr},
    {-1}, {-1}, {-1},
#line 288 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str523, ei_gb2312},
#line 337 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str524, ei_ces_gbk},
#line 276 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str525, ei_jisx0212},
#line 43 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str526, ei_utf32le},
#line 267 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str527, ei_jisx0208},
    {-1}, {-1},
#line 216 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str530, ei_mac_romania},
    {-1},
#line 296 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str532, ei_ksc5601},
    {-1}, {-1}, {-1},
#line 268 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str536, ei_jisx0208},
#line 55 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str537, ei_iso8859_1},
#line 96 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str538, ei_iso8859_6},
#line 44 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str539, ei_utf7},
#line 290 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str540, ei_isoir165},
#line 106 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str541, ei_iso8859_7},
#line 292 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str542, ei_ksc5601},
    {-1},
#line 65 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str544, ei_iso8859_2},
    {-1}, {-1}, {-1},
#line 214 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str548, ei_mac_iceland},
    {-1}, {-1}, {-1},
#line 46 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str552, ei_utf7},
#line 32 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str553, ei_ucs2le},
    {-1}, {-1}, {-1},
#line 249 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str557, ei_cp874},
#line 49 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str558, ei_ucs4internal},
    {-1}, {-1}, {-1}, {-1},
#line 295 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str563, ei_ksc5601},
    {-1},
#line 37 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str565, ei_ucs4le},
    {-1},
#line 275 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str567, ei_jisx0212},
    {-1},
#line 367 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str569, ei_iso2022_kr},
    {-1},
#line 360 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str571, ei_euc_kr},
#line 45 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str572, ei_utf7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 305 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str581, ei_eucjp_ms},
    {-1},
#line 170 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str583, ei_koi8_ru},
    {-1}, {-1},
#line 359 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str586, ei_euc_kr},
#line 110 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str587, ei_iso8859_7},
#line 302 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str588, ei_eucjp_ms},
#line 173 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str589, ei_cp1250},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 301 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str596, ei_eucjp_ms},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 335 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str604, ei_euc_cn},
#line 118 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str605, ei_iso8859_8},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 303 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str611, ei_eucjp_ms},
    {-1}, {-1},
#line 222 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str614, ei_mac_arabic},
    {-1}, {-1}, {-1}, {-1},
#line 284 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str619, ei_iso646_cn},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 18 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str632, ei_ascii},
    {-1}, {-1}, {-1},
#line 17 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str636, ei_ascii},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 364 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str644, ei_johab},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1},
#line 361 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str685, ei_euc_kr},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 273 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str693, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 105 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str699, ei_iso8859_7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 256 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str707, ei_tcvn},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 48 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str713, ei_ucs2swapped},
    {-1},
#line 28 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str715, ei_ucs2be},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1},
#line 213 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str756, ei_mac_centraleurope},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 194 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str765, ei_cp1257},
    {-1},
#line 50 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str767, ei_ucs4swapped},
    {-1}, {-1},
#line 27 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str770, ei_ucs2be},
    {-1}, {-1}, {-1},
#line 220 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str774, ei_mac_turkish},
#line 39 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str775, ei_utf16be},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 42 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str785, ei_utf32be},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1},
#line 188 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str799, ei_cp1255},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 111 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str810, ei_iso8859_7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 228 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str818, ei_nextstep},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 36 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str824, ei_ucs4be},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 358 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str834, ei_big5hkscs},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 357 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str849, ei_big5hkscs},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 185 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str859, ei_cp1254},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 191 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str880, ei_cp1256},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 221 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str898, ei_mac_hebrew},
#line 353 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str899, ei_ces_big5},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 352 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str914, ei_ces_big5},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 218 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str934, ei_mac_ukraine},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1},
#line 299 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str957, ei_euc_jp},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 219 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str1050, ei_mac_greek},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 182 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str1060, ei_cp1253},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 300 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str1111, ei_euc_jp}
  };

#ifdef __GNUC__
__inline
#endif
const struct alias *
aliases_lookup (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = aliases_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int o = aliases[key].name;
          if (o >= 0)
            {
              register const char *s = o + stringpool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &aliases[key];
            }
        }
    }
  return 0;
}
