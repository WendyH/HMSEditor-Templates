///////////////////////////////////////////////////////////////////////////////
// ---- Создание информационной ссылки ----------------------------------------
Function AddInfoItem(aTitle: String): THmsScriptMediaItem;
Begin
  Result := HmsCreateMediaItem('-Info'+IntToStr(PodcastItem.ChildCount), PodcastItem.ItemID);
  Result[mpiTitle     ] := aTitle;  // Наименование (Отображаемая информация)
  Result[mpiTimeLength] := 1;       // Т.к. это псевдо ссылка, то ставим длительность 1 сек.
  Result[mpiThumbnail ] := 'http://wonky.lostcut.net/vids/info.jpg'; // Ставим иконку информации
  Result[mpiCreateDate] := DateTimeToStr(IncTime(gStart, 0, 0, -gnTotalItems, 0));
  Inc(gnTotalItems);
End;
