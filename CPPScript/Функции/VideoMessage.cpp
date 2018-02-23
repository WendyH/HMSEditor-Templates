///////////////////////////////////////////////////////////////////////////////
// Вывод вместо видео заданного сообщения
void VideoMessage(string sCaption, string sMessage) {
  string sFileImage = HmsTempDirectory+'\\videomessage.jpg'; char sCmd;
  sCaption = HmsHttpEncode(ReplaceStr(sCaption, '\n', '|'));
  sMessage = HmsHttpEncode(ReplaceStr(sMessage, '\n', '|'));
  HmsDownloadURLToFile('http://wonky.lostcut.net/videomessage.php?caption='+sCaption+'&msg='+sMessage, sFileImage);
  char sFileMP3 = HmsTempDirectory+'\\silent.mp3';
  try {
    if (!FileExists(sFileMP3)) HmsDownloadURLToFile('http://wonky.lostcut.net/mp3/silent.mp3', sFileMP3);
    sFileMP3 = '-i "'+sFileMP3+'"';
  } except {
    sFileMP3 = '';
  }
  sCmd = Format('%s -loop 1 -f image2 -i "%s" -t %d -r 25 ', [ExtractShortPathName(sFileMP3), ExtractShortPathName(sFileImage), 7]);
  MediaResourceLink = sCmd;
}
