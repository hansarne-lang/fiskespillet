# Første retur til marinaen

Etter ankomst til fiskeplassen kan kapteinen trykke R for å starte hjemreisen. Uferdige fiskekamper avsluttes, og allerede landede fisk beholdes som øktstatus. Serveren flytter båten tilbake langs den samme ruten og registrerer ankomst ved testpunktet ved brygga. Marinasonen og klassifisering mot returfristen følger #16.

I Development-bygget kan både vert og klient startes med -FiskLanSmoke. Bruk samme lokale tilkobling som i [LAN_DAY.md](LAN_DAY.md). Prøven bruker de vanlige spillkommandoene og normal dagsklokke. Kontroller at verten logger én kamp, én fisk klar til landing og én landing for hver av de to spillerne. Begge klientene prøver å lande før fisken er klar og to ganger etter landing. Gjesten prøver hjemreise først. Vertens logg skal vise at turen starter etter kapteinens forsøk, og begge logger skal bekrefte ankomst med 2/2 om bord.
