// ---- Раскодирование строки Uppod плеера ------------------------------------
Function DecodeUppodText(sData: String): String;
Var
  char1, char2: String; i: Integer;
  Client_codec_a: Variant = ["l", "u", "T", "D", "Q", "H", "0", "3", "G", "1", "f", "M", "p", "U", "a", "I", "6", "k", "d", "s", "b", "W", "5", "e", "y", "="];
  Client_codec_b: Variant = ["w", "g", "i", "Z", "c", "R", "z", "v", "x", "n", "N", "2", "8", "J", "X", "t", "9", "V", "7", "4", "B", "m", "Y", "o", "L", "h"];
Begin
  sData := ReplaceStr(sData, "\n", "");
  For i:=0 to Length(Client_codec_a)-1 Do Begin
    char1 := Client_codec_b[i];
    char2 := Client_codec_a[i];
    sData := ReplaceStr(sData, char1, "___");
    sData := ReplaceStr(sData, char2, char1);
    sData := ReplaceStr(sData, "___", char2);
  End;
  Result := HmsUtf8Decode(HmsBase64Decode(sData));
End;