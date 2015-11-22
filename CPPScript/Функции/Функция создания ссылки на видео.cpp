///////////////////////////////////////////////////////////////////////////////
// ---- Создание ссылки на видео ----------------------------------------------
THmsScriptMediaItem AddMediaItem(THmsScriptMediaItem Folder, string sTitle, string sLink, string sGrp='') {
  THmsScriptMediaItem Item = HmsCreateMediaItem(sLink, Folder.ItemID, sGrp); // Создаём ссылку
  Item[mpiTitle     ] = sTitle;      // Наименование
  Item[mpiTimeLength] = gnTime;      // Длительность (тут секунды)
  Item[mpiThumbnail ] = mpThumbnail; // Картинку устанавливаем, которая указана у текущей папки
  Item[mpiCreateDate] = DateTimeToStr(IncTime(gStart, 0, 0, -gnTotalItems, 0));
  gnTotalItems++;                    // Увеличиваем счетчик созданных элементов
  return Item;                       // Возвращаем созданный объект
}
