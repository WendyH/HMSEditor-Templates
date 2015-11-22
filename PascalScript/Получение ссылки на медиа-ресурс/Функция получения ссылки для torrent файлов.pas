// ---- Формирование ссылки для просмотра torrent файла -----------------------
Procedure GetTorrentMediaResourceLink(sFile: String; nIndex: Integer = 1);
Var
  sDownloadsDir, sVal, sLink: String;
Begin
  sDownloadsDir := HmsTranscodingTempDirectory + "Torrents";
  sLink := Format('torrent:%s?index=%d&savepath=%s', [sFile, nIndex, sDownloadsDir]);
  if HmsRegExMatch('--portbegin=(\\d+)', mpPodcastParameters, sVal) Then sLink := sLink+'&portbegin='+sVal;
  if HmsRegExMatch('--portend=(\\d+)'  , mpPodcastParameters, sVal) Then sLink := sLink+'&portend='  +sVal;
  if HmsRegExMatch('--sslport=(\\d+)'  , mpPodcastParameters, sVal) Then sLink := sLink+'&sslport='  +sVal;
  MediaResourceLink := sLink;        
End;
