////////////////////////////////////////////////////////////////////////////////
// Получение ссылки видео vimeo.com
bool GetLink_VimeoCom(string sLink) {
  string sData, sID; bool bSuccess = false;
  
  if (HmsRegExMatch('vimeo.com/(\\d+)', sLink, sID)) sLink = 'http://player.vimeo.com/video/'+sID;
  
  sData = HmsRemoveLineBreaks(HmsDownloadUrl(sLink, 'Referer: '+sLink, true));
  if (HmsRegExMatch('"video":.*?"duration":(\\d+)', sData, sID)) {
    PodcastItem[mpiTimeLength] = HmsTimeFormat(StrToInt(sID))+'.000';
  }
  
  if      (HmsRegExMatch('video/mp4[^}]+"url":"(.*?)"', sData, MediaResourceLink)) bSuccess = true;
  else if (HmsRegExMatch('"hls":{"all":"(http.*?)"'   , sData, sLink)) { MediaResourceLink = ' '+sLink; bSuccess = true; }
  else if (HmsRegExMatch('"hls":{"url":"(http.*?)"'   , sData, sLink)) { MediaResourceLink = ' '+sLink; bSuccess = true; }
  else HmsLogMessage(2, "Не удалось обнаружить ссылку на поток по ссылке c Vimeo.com");
  
  return true;
}
