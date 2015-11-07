// ---- Получение ссылки на ресурс из torrent файла через AceStream -----------
void GetLink_AceStreamTorrent(String sFile) {
  String sCmd, sPlayer;
  sCmd    = 'cmd://"%s" --no-crashdump --play-and-exit --language en -Idummy --demuxdump-file="<OUTPUT FILE>" vlc://pause:4 --access=p2p_access "%s" :demux=dump';
  sPlayer = ExtractShortPathName(RegistryRead('Software\\AceStream\\InstallDir'))+'\\Player\\ace_player.exe';
  MediaResourceLink = Format(sCmd, [sPlayer, sFile]);
}