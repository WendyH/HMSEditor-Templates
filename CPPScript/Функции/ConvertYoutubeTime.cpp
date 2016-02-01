///////////////////////////////////////////////////////////////////////////////
// Конвертация продолжительности из формата youtube (ISO8601) в формат HMS
string ConvertYoutubeTime(string sTime) {
  string sVal; int nSeconds = 0;
  if (HmsRegExMatch('(\\d+)H', sTime, sVal)) nSeconds += StrToInt(sVal)*3600;  
  if (HmsRegExMatch('(\\d+)M', sTime, sVal)) nSeconds += StrToInt(sVal)*60;  
  if (HmsRegExMatch('(\\d+)S', sTime, sVal)) nSeconds += StrToInt(sVal);
  if (nSeconds==0) nSeconds = 600;
  return HmsTimeFormat(nSeconds)+'.000';
}
