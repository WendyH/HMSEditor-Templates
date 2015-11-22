///////////////////////////////////////////////////////////////////////////////
// Получение название группы из имени 
Function GetGroupName(aName: String): String;
Begin
  Result := '#';
  If HmsRegExMatch('([A-ZА-Я0-9])', aName, Result, 1, PCRE_CASELESS) Then Result := Uppercase(Result);
  If HmsRegExMatch('[0-9]', Result, '') Then sGrp := '#';
  If HmsRegExMatch('[A-Z]', Result, '') Then sGrp := 'A..Z';
End;
