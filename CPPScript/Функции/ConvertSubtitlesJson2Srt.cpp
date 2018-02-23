////////////////////////////////////////////////////////////////////////////////
// Конвертация субтитров в формате JSON в srt формат
string ConvertSubtitlesJson2Srt(char sJsonData, int nIntroDuration=0) {
  string sData="", sText, sStart, sEnd; TJsonObject jObject, JSON = TJsonObject.Create();
  int i, nStart;
  try {
    JSON.LoadFromString(sJsonData);
    for(i=0; i<JSON["captions"].Count; i++) {
      jObject = JSON["captions"].AsArray[i];
      nStart = jObject.I["startTime"]+nIntroDuration;
      sStart = HmsTimeFormat(Int(nStart/1000))+',000';
      sEnd   = HmsTimeFormat(Int((nStart+jObject.I["duration"])/1000))+',000';
      sText  = jObject.S["content"];
      sData += Format("%d\r\n%s --> %s\r\n%s\r\n\r\n", [i+1, sStart, sEnd, sText]);
    }
  } finally {
    JSON.Free();
  }
  return sData;
}
