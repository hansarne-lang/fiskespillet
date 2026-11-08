# Samlet teknisk fiskedag

Verten starter Windows-bygget med /Engine/Maps/Entry?listen, og klienten bruker vertens LAN-adresse. Test på samme PC bruker 127.0.0.1. Begge går om bord med E. Fra 02:00 starter kapteinen turen med R. Ved fiskeplassen setter hver spiller ut stanga med F. Venstre museknapp løfter og senker, høyre sveiver, og mellomrom lander når fisken er klar. R som kaptein starter hjemturen.

Den grønne sirkelen rundt brygga er marinasonen. Serveren registrerer båtens midtpunkt ved innkryssing. Den faste ruten er lineær, så kryssingstidspunktet beregnes fra starttid, avstand, fart og radius. Dette gir entydig ankomst også når kryssing og oppgjør behandles i samme ramme. Radius er fem meter i denne prøven.

Dag, reise og napp måles mot serverens monotone klokke. Klientene mottar tidsmålinger fra serveren. Resultatet holdes skjult til 17:00, og kvalifisert fangst gir da en oppdatering av øktsaldoen. Sen retur gir tap av hele dagens fangst. Testfisken representerer én kg til ti fiskepenger. Øktsaldoen starter på null. Disse verdiene brukes til teknisk utprøving, og permanent karriere følger separat.

Start begge Development-prosessene med -FiskLanSmoke for automatisk lokal kjøring med normal dagsklokke. Testen går fra venting på to spillere, gjennom to landinger og retur, til felles avsløring ved 17:00 og dagsslutt ved 20:00. Den bruker egne serverkommandoer og krever ingen tastatur- eller musestyring. Bruk -nullrhi -nosound -unattended for grafikkfri kjøring. Kjør Unreal-automatisering med Fiskelaget.FullDay og Fiskelaget.DayCycle for grense- og oppgjørstestene.

Word-utgaven av kravgrunnlaget oppdateres ved neste samlede dokumentleveranse. Avklaringen om marinasone er nå registrert i Markdown og DECISIONS.
