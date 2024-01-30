Najvisih 6816 bajtova RAMa koriste ROM rutine. ROM bios postavlja SP odmah ispod toga.

RAM je u tri banke po 128k. Moguce ga je podeliti u regione, radi definisanja prava pristupa.
16k kes se koristi pri pristupu flashu. Kes linija je 32 bajta.

Flash se moze podeliti u 4 regiona i za svaki definisati prava pristupa, ali ovo NE radi.
Virtuelni adresni prostor se moze podeliti u 2x4 regiona, samo radi definisanja prava pristupa.
Pri tome prvi i poslednji region su zabranjeni. Ovo radi.
MMU tabela ima 128 ulaza i tako se preslikava 8 MB adresnog prostora u blokovima od 64k.
MMU je deljen za I i D adresni prostor, tako da nije tacno da moze 16MB kao sto pise u pdf.

Interapt kontroler se vrlo fleksibilno steluje u smislu koji IRQ podize pri kom HW dogadjaju.
Interno ima prioritete 0-15, koji se steluju. CPU ima 32 interapta. Nulti je rezervisan za izuzetke.
CPU ne podrzava prioritete (samo M nivo).

Sadrzaj ROM-a se moze naci ovde: https://gist.github.com/motokultivator/53bea80c654d5028b3df797deea34abf

Van radionice:
1. Izmeriti razliku u brzini LD/SD kada su kod i data u istoj banci, odnosno u raznim bankama.
2. Izmeriti da li ima razlike kada se podaci prepisuju iz jedne u drugu banku i u okviru jedne banke.

  Treba zakljuciti kako je najbolje organizovati mem. Nije potpuno jasno do koje mere su I i D bus razdvojeni. Da li
  mogu npr. dva citanja iz jedne banke da se dogode istovremeno ?
  Posto je pipeline, ocekuje se da u svakom ciklusu postoji potreba za read na I bus.

4. Da li nam treba malloc(), i ako da, kako radi ovaj iz ROM-a ?
5. Registri za pisanje po flash-u (preko SPI). Da li postoji mogucnost da se doda SPI RAM ?
6. DMA mem -> mem, sa interaptom.

Radionica:
1. Izmeriti max brzinu promene GPIO.
2. DMA mem -> SPI/I2S.
3. DMA AD -> mem.
