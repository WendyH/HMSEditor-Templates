////////////////////////////////////////////////////////////////////////////////
// Получение ссылки на rutube.ru
void GetLink_RuTube(string sLink) {
  string sData, sVideoId = ""; TJsonObject JSON, BALANCER;
  string sHeaders = "Accept-Encoding: gzip, deflate\r\n"+
                    "Cache-Control: no-cache\r\n"+
                    "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n";
  
  // Восполняем неуказанный протол http, если нужно
  if (LeftCopy(sLink, 2)=="//"  ) sLink = "http:"   + Trim(sLink);
  if (LeftCopy(sLink, 4)!="http") sLink = "http://" + Trim(sLink);
  sLink = ReplaceStr(sLink, "http:", "https:");
  
  HmsRegExMatch("/embed/(\\d+)"                  , sLink, sVideoId);
  HmsRegExMatch("video.rutube.ru/([\\w\\d]{20,})", sLink, sVideoId);
  HmsRegExMatch("rutube.ru/video/([\\w\\d]{20,})", sLink, sVideoId);
  if (sVideoId == "") {
    sData = HmsDownloadUrl(sLink, sHeaders, true);
    HmsRegExMatch("video.rutube.ru/([\\w\\d]{20,})", sData, sVideoId);
    HmsRegExMatch("rutube.ru/video/([\\w\\d]{20,})", sData, sVideoId);
  }
  if (sVideoId != "") {
    sLink = "https://rutube.ru/play/embed/"+sVideoId;
    sHeaders += "Content-Type: application/json\r\n"+
                "Referer: "+sLink+"\r\n";
    sLink = "https://rutube.ru/api/play/options/" + sVideoId;
    sLink+= "?format=json&no_404=true&sqr4374_compat=1&referer=" + HmsHttpEncode(sLink) + "&_="+ReplaceStr(VarToStr(random), ",", ".");
    sData = HmsDownloadUrl(sLink, sHeaders, true);
    if (HmsRegExMatch("(http[^\">']+m3u8[^\"}>']+)", sData, sLink)) {
      MediaResourceLink = " "+sLink;
    } else {
      JSON  = TJsonObject.Create();
      try {
        JSON.LoadFromString(sData);
        sImg = JSON.S["thumbnail_url"];
        if (sImg!="") PodcastItem[mpiThumbnail] = sImg;
        BALANCER = JSON.O["video_balancer"];
        if (BALANCER!=nil) {
          sLink = BALANCER.S["json"];
          if (sLink!="") {
            sData = HmsDownloadUrl(sLink, sHeaders, true);
            HmsRegExMatch("(http[\">']+)", sData, MediaResourceLink);
          }
        }
      } finally { JSON.Free; }
    }
  }
  if (MediaResourceLink=="")
    HmsLogMessage(2, mpTitle+": не удалось получить ссылку на видео rutube.ru по "+sLink);
}
