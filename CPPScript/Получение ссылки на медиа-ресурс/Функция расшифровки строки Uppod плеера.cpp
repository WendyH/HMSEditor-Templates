// ---- Раскодирование строки Uppod плеера ------------------------------------
string DecodeUppodText(string sData) {
  char char1, char2; int i;
  variant Client_codec_a = ["l", "u", "T", "D", "Q", "H", "0", "3", "G", "1", "f", "M", "p", "U", "a", "I", "6", "k", "d", "s", "b", "W", "5", "e", "y", "="];
  variant Client_codec_b = ["w", "g", "i", "Z", "c", "R", "z", "v", "x", "n", "N", "2", "8", "J", "X", "t", "9", "V", "7", "4", "B", "m", "Y", "o", "L", "h"];

  sData = ReplaceStr(sData, "\n", "");
  for (i=0; i<Length(Client_codec_a); i++) {
    char1 = Client_codec_b[i];
    char2 = Client_codec_a[i];
    sData = ReplaceStr(sData, char1, "___");
    sData = ReplaceStr(sData, char2, char1);
    sData = ReplaceStr(sData, "___", char2);
  }
  sData = HmsUtf8Decode(HmsBase64Decode(sData));
  return sData;
}