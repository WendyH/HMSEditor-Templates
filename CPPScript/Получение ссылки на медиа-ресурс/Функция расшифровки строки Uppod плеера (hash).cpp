// ---- Раскодирование строки Uppod плеера с помощью hash ---------------------
string DecodeUppodTextHash(string sData) {
  variant char1, char2, hash, tab_a, tab_b; int i;

  hash = "0123456789WGXMHRUZID=NQVBLihbzaclmepsJxdftioYkngryTwuvihv7ec41D6GpBtXx3QJRiN5WwMf=ihngU08IuldVHosTmZz9kYL2bayE";

  // Проверяем, может не нужно раскодировать (json или ссылка)
  if ((Pos("{", sData)>0) || (LeftCopy(sData, 4)=="http")) return HmsUtf8Decode(sData);

  sData = DecodeUppod_tr(sData, "r", "A");
  
  hash = ReplaceStr(hash, 'ih', '\n');
  if (RightCopy(sData, 1)=='!') {
    sData = LeftCopy(sData, Length(sData)-1);
    tab_a = ExtractWord(4, hash, '\n');
    tab_b = ExtractWord(3, hash, '\n');
  } else {
    tab_a = ExtractWord(2, hash, '\n');
    tab_b = ExtractWord(1, hash, '\n');
  }

  sData = ReplaceStr(sData, "\n", "");
  for (i=1; i<=Length(tab_a); i++) {
    char1 = Copy(tab_b, i, 1);
    char2 = Copy(tab_a, i, 1);
    sData = ReplaceStr(sData, char1, "___");
    sData = ReplaceStr(sData, char2, char1);
    sData = ReplaceStr(sData, "___", char2);
  }
  sData = HmsUtf8Decode(HmsBase64Decode(sData));
  sData = ReplaceStr(sData, "hthp:", "http:");
  return sData;
}

string DecodeUppod_tr(string sData, string ch1, string ch2) {
  string s = ""; int i, loc3, nLen;

  if ((Copy(sData, Length(sData)-1, 1)==ch1) && (Copy(sData, 3, 1)==ch2)) {
    nLen = Length(sData);
    for (i=nLen; i>0; i--) s += Copy(sData, i, 1);
    loc3 = Int(StrToIntDef(Copy(s, nLen-1, 2), 0)/2);
    s = Copy(s, 3, nLen-5); i = loc3;
    if (loc3 < Length(s)) {
      while (i < Length(s)) {
        s = LeftCopy(s, i) + Copy(s, i+2, 99999);
        i+= loc3;
      }
    }
    sData = s + "!";
  }
  return sData;
}