TRURLib

Jest to zacz±tek biblioteki, która bêdzie u¿ywana w ca³ym programie
instalacyjnym. 

1. Zawarto¶æ

   a) xmallocs czyli opakowania dla funkcji [ma,ca,rea]lloc, strdup oraz free.

   Funkcje te s± redefiniowane, tak¿e nie trzeba (nie wolno) u¿ywaæ wprost 
   wersji x*, a jedynie w³±czyæ nag³ówek <trurl/xmalloc.h> przy jednoczesnym 
   -DUSE_XMALLOCS w Makefile. Przy braku tej definicji wo³ane s± funkcje 
   orginalne.   

   Modu³ przeniesiony z pldilib, gdzie wystêpowa³ pod t± sam± nazw±. 
   Nag³ówek <trurl/xmallocs.h> 
   
 	     	 
   b) makroinstrukcja n_assert(), analogiczna do assert(), z tym, ¿e
      zamiast abort() wo³aj±ca funkcjê wskazan± przez nas.

   W kodzie _nie_ wolno u¿ywaæ wprost assert()	
   Modu³ przeniesiony z pldilib, gdzie wystêpowa³ pod nazw± massert,
   stara nazwa makroinstrukcji(m_assert) zosta³a zachowana, ze wzglêdu
   na kompatybilno¶æ wstecz.  

   Nag³ówek <trurl/nassert.h> 
              

   c) Parê u¿ytecznych struktur danych:

      - samo realokuj±ca siê tablica wska¼ników na cokolwiek 
        <trurl/narray.h>  

      - lista wska¼ników na cokolwiek
        <trurl/nlist.h>  	

      - hash (key, value) 
        <trurl/nhash.h> 	

      - dbhash (key, value) - opakowanie dla funkcji libdb (man dbopen) 
        <trurl/ndbhash.h> 		 

      - string, nie skoñczony, nie nadaje siê do u¿ytku. 

   d) Operacje na stringach (pliki nstr_NAZWAFUNKCJI.c)
      Nag³ówek <trurl/nstr.h> 
   

Wszystkie prototypy funkcji s± dostêpne tak¿e w nag³ówku ,,g³ównym'' 
<trurl/trurl.h>.

Proszê o _nie_ w³±czanie poszczególnych nag³ówków z u¿yciem ¶cie¿ek 
wzglêdnych. Niech to zawsze bêdzie
#include <trurl/nag³owek.h> + '-Ikatalog'  
  

2.  Konwencje nazewnicze
 
   - nazwy funkcji zaczynaj± siê od 'n_'

   - nazwy typów strukturalnych: tn_NAZWATYPU, np. tn_array   

   - nazwy funkcji operuj±cych na typach n_NAZWATYPU_NAZWAFUNKCJI, 
      np. n_array_sort()  

   - nazwy typów funkcji zaczynaj± siê od 't_fn' 


3. B³êdy 

 - ca³o¶æ nie jest do koñca stestowana. Je¿eli dana funkcja nie 
   jest wywo³ana w test_*.c, to znaczy, ¿e nikt tego nie testowa³
  
 - nstring.c jest nieukoñczony i _nie_ nadaje siê do u¿ytku.

 

Przk³ady u¿ycia w test_* 

Pytania, sugestie, etc >> pld-installer@pld.org.pl

Pawe³ Gajda (mis@k2.net.pl)



