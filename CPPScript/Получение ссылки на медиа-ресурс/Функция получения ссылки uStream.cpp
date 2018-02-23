////////////////////////////////////////////////////////////////////////////////
// Получение ссылки на ustream.tv
bool GetLink_UstreamTv(string sLink) {
  string sHtml, sMgid, sVideoId, sStreamName, sCdnUrl, siLink, sAmfLink;
  siLink   = 'http://iphone-streaming.ustream.tv/uhls/%s/streams/live/iphone/playlist.m3u8';
  sAmfLink = 'http://cdngw.ustream.tv/Viewer/getStream/1/%s.amf';
  if (HmsRegExMatch('ustream.tv/\\w+/(\\d+)$', sLink, sVideoId)) {
    // Recorded
    sHtml = HmsDownloadUrl(sLink, 'Referer: '+sLink, true);
    if (HmsRegExMatch(sVideoId+'(\\?preset_id[^"\']+)', sHtml, sLink))
      sVideoId = sVideoId + HmsHtmlDecode(sLink);
    MediaResourceLink = 'http://tcdn.ustream.tv/video/'+sVideoId;
    return true;
  } else {
    if (!HmsRegExMatch('ustream.tv/(\\d+)$', sLink, sVideoId)) {
      sHtml = HmsDownloadUrl(sLink, 'Referer: '+sLink, true);
      if (!HmsRegExMatch('contentId=(\\d+)', sHtml, sVideoId)) {
        HmsLogMessage(2, mpTitle+' Не смог найти VideoId в ссылке ustream.tv');
        return true;
      }
      if (HmsRegExMatch('"stream":{[^}]*"hls":"(.*?)"', sHtml, sLink)) {
        MediaResourceLink = ' '+HmsJsonDecode(sLink);
        return true;
      }
    }
  }
  siLink = Format(siLink, [sVideoId]); sAmfLink = Format(sAmfLink, [sVideoId]);
  // Live
  sHtml = HmsDownloadUrl(sAmfLink, 'Referer: '+sLink, true);
  //HmsStringToFile(sHtml, 'D:\\'+sVideoId+'.amf');
  sLink = '';
  if (Pos('fmsUrl', sHtml)>0) {
    HmsRegExMatch('fmsUrl.*?(rtmp:[^\\x00]+)', sHtml, sCdnUrl);
    if (sCdnUrl=='') {HmsLogMessage(2, mpTitle+' Не смог получить fmsUrl в amf файле. Канал отключен?'); return true;}
    sLink = Format('rtmpdump.exe -v -W "http://www.ustream.tv/flash/viewer.swf" -r "%s" -a "ustreamVideo/%s" -y "streams/live" -p "%s"', [sCdnUrl, sVideoId, sLink]);
  } else if (Pos('cdnUrl', sHtml)>0) {
    HmsRegExMatch('cdnUrl.*?(rtmp:[^\\x00]+)', sHtml, sCdnUrl);
    HmsRegExMatch('streamName.*?(\\w+)', sHtml, sStreamName);
    HmsRegExMatch('(http://www.ustream.tv/channel/[^\\x00]+)', sHtml, sLink);
    if (sCdnUrl=='') {HmsLogMessage(2, mpTitle+' Не смог найти cdnUrl в amf файле. Канал отключен?'); return true;}
    if (RightCopy(sCdnUrl, 1)=='/') sCdnUrl = LeftCopy(sCdnUrl, Length(sCdnUrl)-1);
    sLink = Format('rtmpdump.exe -v -W "http://www.ustream.tv/flash/viewer.swf" -r "%s/%s" -p "%s"', [sCdnUrl, sStreamName, sLink]);
  } else if (Pos('EXTM3U', HmsDownloadUrl(siLink, '', true))>0) {
    sLink = ' '+siLink;
  } else {
    // Канал отключен от прямого вещания? пробуем показать то было в записи
    sLink = 'http://tcdn.ustream.tv/video/'+sVideoId;
  }
  MediaResourceLink=sLink;
  return true;
}
