///////////////////////////////////////////////////////////////////////////////
// Формирование параметров запуска hdsdump для воспроизведения по манифесту f4m
Procedure HDSResourceLink(sManifest, sReferer, sCookies:String='');
Var
  sQuality: String;
Begin 
  MediaResourceLink := 'cmd://"'+ProgramPath+'\Transcoders\hdsdump.exe" --manifest "' + sManifest + '" -o "<OUTPUT FILE>"';
  if cfgTranscodingThreadCount > 1 then MediaResourceLink := MediaResourceLink + ' --threads '+Str(cfgTranscodingThreadCount);
  if sReferer<>'' then MediaResourceLink := MediaResourceLink + ' --referer "' + sReferer + '"';
  if sCookies<>'' then MediaResourceLink := MediaResourceLink + ' --cookies "' + sCookies + '"';
  HmsRegExMatch('--quality=(\w+)',  mpPodcastParameters, sQuality); // --quality=low|medium|high
  if sQuality<>'' then MediaResourceLink := MediaResourceLink + ' --quality ' +  sQuality;
  if mpTimeStart<>'' then MediaResourceLink := MediaResourceLink + ' --skip ' + mpTimeStart;
End;
