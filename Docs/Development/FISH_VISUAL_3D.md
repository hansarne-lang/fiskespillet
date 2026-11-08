# Enkel 3D-fisk

Bygger videre på 3D-turen fra VISUAL_3D.md. Hver stang har sin egen generelle fiskemodell med kropp, finner, øyne og bevegelig hale. Fisken blir synlig nær båten når ti meter eller mindre line gjenstår, og ved klarsignal til landing. Lina festes til fiskens munn mens den er synlig. Ved landing løftes fisken inn på dekket og vises i en kort animasjon på tre sekunder.

Serveren deler visningsfase, linelengde og det opprinnelige landingstidspunktet. Gjentatt landingsforsøk starter ikke animasjonen på nytt. Bekreftede resultater og økonomi følger den eksisterende oppgjørsbarrieren ved 17:00. Modellen illustrerer en generell prototypefisk.

Test med to lokale Development-prosesser og -FiskLanSmoke. Legg til -FiskScreenshotDir=<lokal mappe> for bildeprøve. fish-near-boat.png viser egen fisk ved båten, og fish-landing.png viser landingen. Kontroller begge spillernes bilder og loggmeldingene FISKELAGET_FISH_VISUAL_NEAR, FISKELAGET_FISH_VISUAL_LANDING og FISKELAGET_FISH_VISUAL_FINISHED. Prøven skal fortsatt registrere nøyaktig én landet fisk per spiller, selv med gjentatte landingsforsøk.

Claude-gjennomgangen venter til lørdag.
