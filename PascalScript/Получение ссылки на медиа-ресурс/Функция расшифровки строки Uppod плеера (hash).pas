// ---- Раскодирование строки Uppod плеера с помощью hash ---------------------
Function DecodeUppodTextHash(sData: String): String;
Var
  char1, char2, hash, tab_a, tab_b: Variant; i: Integer;
Begin
  hash := "0123456789WGXMHRUZID=NQVBLihbzaclmepsJxdftioYkngryTwuvihv7ec41D6GpBtXx3QJRiN5WwMf=ihngU08IuldVHosTmZz9kYL2bayE";

  // Проверяем, может не нужно раскодировать (json или ссылка)
  if (Pos('{', sData)>0) OR (LeftCopy(sData, 4)='http') then Result := HmsUtf8Decode(sData);

  sData := DecodeUppod_tr(sData, "r", "A");
  
  hash := ReplaceStr(hash, 'ih', '\n');
  if RightCopy(sData, 1)='!' then begin
    sData := LeftCopy(sData, Length(sData)-1);
    tab_a := ExtractWord(4, hash, '\n');
    tab_b := ExtractWord(3, hash, '\n');
  end else begin
    tab_a := ExtractWord(2, hash, '\n');
    tab_b := ExtractWord(1, hash, '\n');
  end;

  sData := ReplaceStr(sData, "\n", "");
  for i:=1 to Length(tab_a) do begin
    char1 := Copy(tab_b, i, 1);
    char2 := Copy(tab_a, i, 1);
    sData := ReplaceStr(sData, char1, "___");
    sData := ReplaceStr(sData, char2, char1);
    sData := ReplaceStr(sData, "___", char2);
  end;
  Result := HmsUtf8Decode(HmsBase64Decode(sData));
End;

Function DecodeUppod_tr(sData, ch1, ch2: String): String;
Var
  s: String=''; i, loc3, nLen: Integer;
Begin
  Result := sData;
  if (sData[Length(sData)-2]=ch1) AND (sData[3]=ch2) then begin
    nLen := Length(sData);
    for i:= nLen-1 downto 0 do s := s + sData[i];
    loc3 := Int(StrToIntDef(Copy(s, nLen-1, 2), 0)/2);
    s := Copy(s, 3, nLen-5); i := loc3;
    if loc3 < Length(s) then
      while i < Length(s) do begin
        s := LeftCopy(s, i) + Copy(s, i+2, 99999);
        i := i + loc3;
      end;
    Result := s + "!";
  end;
End;