* 99.07.12 (v0.1)
- poprawiony b³±d w funkcjach *bsearch z narray.c. £atka Jarka Woloszyna. 
- poprawiony b³±d w nstr_tokl()
- poprawki zwi±zane z glibc-2.1( z RH6.0)

	
* 99.06.04

- poprawiony b³±d (pope³niony podczas poprzedniej poprawki) w n_array_clean() 

- dosyæ istotna zmiana w narray. Modu³ ma teraz w³asne funkcje qsort() 
  i bsearch() - tym samym funkcje cmp_fn() otrzymuj± teraz nie wska¼nik do 
  elementu, lecz (tak samo jak dla nlisty) element. 
  
  Innymi s³owy funkcja cmp() dla tablicy ma postaæ cmpfn(void*, void*), a nie, 
  jak do tej pory cmpfn(void**,void**) 
  

  Stare funkcje zosta³y zachowane pod nazwami: 
  n_array_libc_qsort_ex()   (dawna n_array_sort_ex()) oraz
  n_array_libc_bsearch_ex() (dawna n_array_bsearch_ex()

- nowe funkcje 
  n_array_map() 
  n_array_map_arg() 

- program testowy zosta³ podzielony na kilka czê¶ci.  


* 99.06.02
- poprawiony b³±d w n_array_clean(), ³atka Jarka Woloszyna.
- dosyæ mocno zmieniony ndbhash


* 99.05.28pm
- poprawiony ohydny b³±d w n_array_bsearch() i narray.h. Na podstawie ³atki 
  Jarka Woloszyna <yossa@dione.ids.pl>


* 99.05.28
- poprawionych parê grubszych b³êdów w nhash
- zmiana interfejsu nhash


* 99.05.27
- poprawionych sporo b³êdów w module narray
- poprawione b³êdy w n_list_(shift, pop). Na podstawie ³atek 
  Wojtka Bojdo³a <wojboj@lp.net.pl> 

- nowe funkcje:
  n_list_remove_nth() 

  n_array_bsearch() 
  n_array_uniq()
  n_array_eq() 
  n_array_remove_nth()

- ununiête funkcje
  n_array_search*() (tymczasowo zdefiniowane jako makra)

- zmiany interfejsu nhash.

- 1 wersja modu³u ndbhash, w nastêpnej prawdopodobnie zmieni siê 
  trochê interfejs   





