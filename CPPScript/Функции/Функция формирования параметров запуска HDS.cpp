///////////////////////////////////////////////////////////////////////////////
// Формирование параметров запуска hdsdump для воспроизведения по манифесту f4m
void HDSResourceLink(string sManifest, string sReferer='', string sCookies='') {
  string sQuality;
  
  MediaResourceLink = 'cmd://"'+ProgramPath+'\\Transcoders\\hdsdump.exe" --manifest "' + sManifest + '" -o "<OUTPUT FILE>"';
  if (cfgTranscodingThreadCount > 1) MediaResourceLink += ' --threads '+Str(cfgTranscodingThreadCount);
  if (sReferer!='') MediaResourceLink += ' --referer "' + sReferer + '"';
  if (sCookies!='') MediaResourceLink += ' --cookies "' + sCookies + '"';
  HmsRegExMatch('--quality=(\\w+)',  mpPodcastParameters, sQuality); // --quality=low|medium|high
  if (sQuality!='') MediaResourceLink += ' --quality ' +  sQuality;
  if (mpTimeStart!='') MediaResourceLink += ' --skip ' + mpTimeStart;
}
