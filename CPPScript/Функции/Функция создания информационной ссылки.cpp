///////////////////////////////////////////////////////////////////////////////
// ---- Создание информационной ссылки ----------------------------------------
void AddInfoItem(string sTitle) {
  THmsScriptMediaItem Item = HmsCreateMediaItem('-Info'+IntToStr(PodcastItem.ChildCount), PodcastItem.ItemID);
  Item[mpiTitle     ] = sTitle;  // Наименование (Отображаемая информация)
  Item[mpiTimeLength] = 1;       // Т.к. это псевдо ссылка, то ставим длительность 1 сек.
  Item[mpiThumbnail ] = 'http://wonky.lostcut.net/vids/info.jpg'; // Ставим иконку информации
  Item[mpiCreateDate] = DateTimeToStr(IncTime(gStart, 0, 0, -gnTotalItems, 0));
  gnTotalItems++;
}
