// ---- Формирование ссылки для просмотра torrent файла -----------------------
void GetTorrentMediaResourceLink(String sFile, int nIndex = 1) {
  String sDownloadsDir, sVal, sLink;

  sDownloadsDir = HmsTranscodingTempDirectory + "Torrents";
  sLink = Format('torrent:%s?index=%d&savepath=%s', [sFile, nIndex, sDownloadsDir]);
  if (HmsRegExMatch('--portbegin=(\\d+)', mpPodcastParameters, sVal)) sLink += '&portbegin='+sVal;
  if (HmsRegExMatch('--portend=(\\d+)'  , mpPodcastParameters, sVal)) sLink += '&portend='  +sVal;
  if (HmsRegExMatch('--sslport=(\\d+)'  , mpPodcastParameters, sVal)) sLink += '&sslport='  +sVal;
  MediaResourceLink = sLink;        
}