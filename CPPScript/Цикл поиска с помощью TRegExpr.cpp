RegExp = TRegExpr.Create('<li(.*?)</li>', PCRE_SINGLELINE);
try {
  if (RegExp.Search(sData)) do {
    sName = ""; sLink = "";
    HmsRegExMatch('<a[^>]+href="(.*?)"', RegExp.Match, sLink);
    HmsRegExMatch('(<a.*?</a>)'        , RegExp.Match, sName);
    
    sLink = HmsExpandLink(sLink, gsUrlBase);
    sName = HmsHtmlToText(sName);
    
    //...
   
  } while (RegExp.SearchAgain);
  
} finally { RegExp.Free; }
