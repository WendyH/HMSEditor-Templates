///////////////////////////////////////////////////////////////////////////////
// Создание ссылки на видео
Function AddMediaItem(Folder: THmsScriptMediaItem; sTitle, sLink: String; sGrp: String=''): THmsScriptMediaItem;
Begin
  Result := HmsCreateMediaItem(sLink, Folder.ItemID, sGrp); // Создаём ссылку
  Result[mpiTitle     ] := sTitle;      // Наименование
  Result[mpiTimeLength] := gnTime;      // Длительность (тут секунды)
  Result[mpiThumbnail ] := mpThumbnail; // Картинку устанавливаем, которая указана у текущей папки
  Result[mpiCreateDate] := DateTimeToStr(IncTime(gStart, 0, 0, -gnTotalItems, 0));
  Inc(gnTotalItems); // Увеличиваем счетчик созданных элементов
End;
