# Første LAN-fiskedag

Verten starter Windows-bygget med argumentet `/Engine/Maps/Entry?listen`. Den andre spilleren starter samme bygg med vertens lokale IP-adresse som første argument. Standardporten er UDP 7777. På samme PC kan klienten bruke `127.0.0.1`.

Verten viser en ventemelding til den andre spilleren kobler seg til. Da starter serveren én fiskedag. Begge vinduer skal vise samme fase og omtrent samme klokkeslett. Ved 02:00 vises «Fiske og retur», ved 17:00 «Dagsoppgjør», og ved 20:00 stopper klokken. En ny dag krever en ny økt.

Fasene er foreløpig indikatorer. Båt, stenger, fangst, returregistrering og salg implementeres i egne oppgaver.
