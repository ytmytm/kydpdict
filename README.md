
Kydpdict
========

(c) 2009 Maciej Witkowiak <ytm@elysium.pl>
http://members.elysium.pl/ytm/html/kydpdict.html
http://ytm.bossstation.dnsalias.org/html/kydpdict.html



#1. CO TO JEST

Program Kydpdict jest graficzn� nak�adk� pozwalaj�c� na �atwe i efektywne
korzystanie w �rodowisku graficznym z darmowego i komercyjnych s�ownik�w
dost�pnych pod Windows.
Dzia�a pod systemem Linux i (prawdopodobnie nadal) MacOS X. Wbrew swojej
nazwie nie jest programem zale�nym od �rodowiska KDE. Od wersji 0.9.0
r�wnie� wbrew swojej nazwie nie jest li tylko nak�adk� na s�ownik YDP.

##1.1 WYMAGANIA

Konieczne jest posiadanie biblioteki Qt w wersji >=3.1, je�li kompilujemy
program konieczny b�dzie tak�e pakiet Qt-devel.

##1.2 KOMPILACJA

Wykonaj
```
    ./configure
```
ewentualnie (zobacz ni�ej komu mog�oby si� to przyda�):
```
    ./configure --without-x
```
Wykonaj
```
    make
```

Je�li skrypt ./configure zako�czy si� b��dem, to prawdopodobnie nie mo�e znale�� �cie�ek
do Qt. Nale�y w�wczas jawnie je poda�, np.
```
    ./configure --with-qt-includes=/usr/X11R6/include/qt --with-qt-libs=/usr/X11R6/lib
```
Je�li Qt zosta�o zainstalowane bezpo�rednio ze �r�de� to prawdopodobnie wystarczy co�
podobnego do:
```
    ./configure --with-qt-dir=/usr/local/qt
```

W razie k�opot�w zajrzyj do dokumentacji Qt (to o $QTDIR te� przeczytaj :).

Je�li wywo�asz ./configure --without-x, to program b�dzie bezpo�rednio zale�a� tylko
od biblioteki Qt. Mo�e mie� to zastosowanie na systemach, na kt�rych nie ma X11, ale
jest dost�pne Qt (MacOS X z natywnym Qt (nie-X11)?) oraz u tych puryst�w, kt�rzy nie
chc� w og�le niczego zwi�zanego z ryb� na tacce systemowej.

Instalacja - wykonaj
```
    make install
```

Na FreeBSD mo�e by� konieczne korzystanie z gmake oraz jawne dodanie biblioteki pthread,
mo�na to zrobi� nast�puj�co:

```
CXXFLAGS="$CXXFLAGS -lpthread" ./configure
```


#2. OBS�UGIWANE TYPY S�OWNIK�W


##2.1 SAP

SAP, czyli s�ownik angielsko-polski (i polsko-angielski) to domy�lny modu�
s�ownikowy. Jest najm�odszy, dlatego jeszcze wymaga poprawek, jednak ju�
u�yteczny. Pierwotna, konsolowa, wersja SAP nie ma dzia�aj�cej w�asnej strony
domowej, mo�na go pobra� z zasob�w PLD: http://www.pld-linux.org/ lub st�d:
http://pbs.linux.net.pl/?op=pr&f=sap


##2.2 YDP

Ten modu� obs�uguje pliki danych s�ownik�w wydanych przez Young Digital Poland,
czyli s�ownik�w Collins i Langenscheidt. Do dzia�ania niezb�dne jest posiadanie
plik�w z zainstalowanego pod Windows programu. Dla wersji angielskiej chodzi o
pliki:
    dict100.dat, dict101.dat, dict100.idx, dict101.idx
a dla niemieckoj�zycznej:
    dict200.dat, dict201.dat, dict200.idx, dict201.idx.
Najwygodniej skopiowa� je do domy�lnego katalogu danych, to jest:
    /usr/share/kydpdict lub /usr/local/share/kydpdict
albo umie�ci� tam tylko linki symboliczne do plik�w na partycji windowsowej.
Nazwy plik�w powinny zawiera� tylko ma�e litery.

Do odtwarzania wymowy s��w potrzebny b�dzie zewn�trzny program. Mo�e to by� play
do oryginalnych plik�w wav lub je�li zosta�y przekodowane: mpg123, ogg123 i wiele
innych.
Odtwarzacz jest wsp�lny dla plik�w d�wi�kowych obu j�zyk�w, wi�c musisz zadba�,
aby by�y one w jednakowym formacie.
Je�li korzystasz z serwera d�wi�ku by� mo�e zamiast play trzeba b�dzie u�y�
esdplay lub artsplay. (mplayer si� _nie nadaje_ - �le odtwarza kr�tkie pliki
d�wi�kowe)


##2.3 PWN Oxford 2003

Trzeci modu� s�ownikowy pozwala na korzystanie ze s�ownika PWN Oxford 2003.
Jest to tylko s�ownik angielsko-polski, kt�ry by� do��czany do papierowego
wydania s�ownika Oxford.
Niezb�dne jest posiadanie pliku danych z ju� zainstalowanej aplikacji pod Windows.
W tym wypadku chodzi o plik 'angpol.win'. Dalej nale�y post�powa� podobnie, jak
opisano dla modu�u YDP.


##2.4 PWN Oxford 2004

Odmiana wersji z 2003 roku, obs�uguje s�owniki angielsko-polski i polsko-angielski.
Niezb�dne jest posiadanie obu plik�w danych z ju� zainstalowanej aplikacji pod
Windows. Chodzi o pliki 'angpol.win' oraz 'polang.win'. Dalej nale�y post�powa�
tak, jak opisano wy�ej.



#3. ZALETY PROGRAMU

Korzystanie ze s�ownika nigdy nie by�o tak proste jak obecnie. W wi�kszo�ci
przypadk�w t�umaczymy teksty w postaci elektronicznej. Nie ma wi�c sensu wpisywa�
z klawiatury wyraz�w (czas znacznie powy�ej sekundy), ani nawet zaznacza� tekstu,
naciska� Ctrl+C, zmienia� okno na okno s�ownika i wkleja� tekst naciskaj�c Ctrl+V
(czas rz�du sekundy). Teraz pracuj�c z tekstem obcoj�zycznym (np w przegl�darce www)
wystarczy tylko zaznaczy� wyraz (w wi�kszo�ci aplikacji zaznacza si� tekst dwukrotnym
klikni�ciem na wyrazie) aby pojawi�o si� "na g�rze" okno s�ownika z t�umaczeniem
(czas rz�du u�amka sekundy!). Je�li wyrazu, kt�ry zaznaczy�e� nie ma w s�owniku,
pojawi si� t�umaczenie wyrazu mu najbli�szego (np: wyrazu "investigations" nie
ma w s�owniku, ale najbli�szy mu podobny to "investigation").

Od tej chwili wszystko co zaznaczysz pojawi si� przet�umaczone, je�li zacznie
Ci to przeszkadza� po prostu wy��cz opcj� �ledzenia schowka - przez pasek ikon
lub menu z ryby.

Jedn� z zalet jest tak�e �atwa konfiguracja programu: �cie�ek dost�pu do plik�w
s�ownika, �cie�ki do plik�w z wymow� (oryginalne pliki wav lub zakodowane do
mp3/ogg), a tak�e kolor�w tekstu t�umaczenia i t�a aplikacji. Te i inne parametry
programu (np pozycja na ekranie, wielko�� okna s� przechowywane w katalogu
.kydpdict w Twoim katalogu konfiguracyjnym. B�dzie to katalog okre�lony przez
bezwzgl�dn� �cie�k� ze zmiennej $HOME_ETC lub katalog domowy lub katalog poni�ej
niego okre�lony przez zmienn� $CONFIG_DIR (testowane w tej kolejno�ci).

Kilka najwa�niejszych skr�t�w klawiaturowych:
- ESC pozwala szybko wpisa� z klawiatury nowy wyraz - czy�ci lini� edycji i przenosi tam kursor
- kolejne naci�ni�cie ESC (ale dopiero po up�yni�ciu co najmniej .5s) pozwala ukry� s�ownik - jak przy klikni�ciu na rybie
- Ctrl+` (znak nad TAB) pozwala szybko zamieni� kierunek t�umaczenia



#4. KONFIGURACJA

W razie problem�w po prostu skasuj katalog z plikami konfiguracyjnymi.
Program postara si� odtworzy� go z domy�lnych warto�ci oraz korzystaj�c z
/etc/ydpdict.conf je�li korzystasz jednocze�nie z konsolowego s�ownika ydpdict.

Wyboru modu�u s�ownikowego i �cie�ki do plik�w danych specyficznych dla
tego s�ownika dokonujemy w zak�adce 'S�ownik'. Zmiany zostan� uwzgl�dnione
po ponownym uruchomieniu programu. W danej chwili mo�na mie� aktywny tylko
jeden rodzaj s�ownika.

Kr�tki opis opcji konfiguracyjnych z zak�adek 'R�ne' i 'Schowek'
- 'W��cz �ledzenie schowka' - okno s�ownika wyskakuj�ce na wierzch mo�e przeszkadza�,
   w ten spos�b mo�na wy��czy� takie zachowanie
- 'Ignoruj w�asne zaznaczenia' - je�li zawarto�� schowka pochodzi z okna t�umacze�
   kydpdict, to zostanie zignorowana (kto� ma lepszy pomys� na nazw� tej opcji?)
- 'U�ywaj pochy�ej czcionki' - niekt�re czcionki wygl�daj� brzydko w odmianie
   pochy�ej; mo�na nieco poprawi� wygl�d i czytelno�� okna wy��czaj�c je t� opcj�
- 'W��cz podpowiedzi' - opcja kontroluj�ca wy�wietlanie wyja�nie� po najechaniu
   myszk� na s�ownikowe skr�ty
- 'W��cz ikon� na tacce systemowej' - pozwala wy��czy� ikon� dla manager�w okien,
   z kt�rymi sprawia to problemy
- 'Autoodtwarzanie d�wi�ku' - dotyczy tylko tekstu przechwyconego ze schowka
- 'Ustawiaj focus na siebie' - czasem wygodnie jest mie� okno Kydpdict wyskakuj�ce
   na wierzch z t�umaczeniem ale nie pozostawiaj�ce jako aktywne poprzednie okno - ta
   opcja kontroluje takie zachowanie (wyj�tek: je�li okno by�o niewidoczne - po
   trzykrotnym ESC, to zawsze dostanie focus)

D�u�szego om�wienia wymaga opcja 'Wy�wietlaj znaki Unicode'. Po jej w��czeniu,
znaki transkrypcji fonetycznej zamiast pochodzi� z wbudowanych plik�w graficznych
(kt�re nie zmieniaj� rozmiaru wraz z fontem) b�d� konwertowane na odpowiednie kody
Unicode.
Oznacza to, �e do prawid�owego wy�wietlania niezb�dne jest posiadanie fontu, kt�ry
takie znaki posiada. Obecnie pod Linuksem nie ma ju� wi�kszego problemu z instalacj�
nowych font�w TTF, dlatego opis instalacji pomin� i nie b�d� odpowiada� na pytania
w tej sprawie :). Oto kilka adres�w, z kt�rych mo�na �ci�gn�� darmowe fonty
obs�uguj�ce potrzebny w s�owniku zakres znak�w:

Lucida Sans Unicode:
http://www.phon.ucl.ac.uk/home/wells/lsansuni.ttf

Doulos:
http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=DoulosSILfont

W poni�szych brakuje znaku strza�ki w prawo - odsy�acza, poza tym s� bez zarzutu:

Caslon:
http://bibliofile.mc.duke.edu/gww/fonts/Unicode.html

Gentium:
http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=gentium



#5. Q & A (jeszcze nie FAQ, bo to pierwsza edycja :)

```
Q: Program nie potrafi zapami�ta� swojego po�o�enia na ekranie
A: Najprawdopodobniej wymusza to tw�j manager okien, Kydpdict nic na to nie
   poradzi.

Q: W KDE program nie wyskakuje ponad inne okna po znaznaczeniu nowego wyrazu
A: W Centrum Sterowania->Pulpit->Okna->Zaawansowane zmie� poziom ochrony przed
   zmian� aktywnego okna na 'Brak'.

Q: Skr�ty klawiaturowe s� chore - kto to wymy�la�?
A: Ja. Je�li masz pomys� na �atwiejsze, nie koliduj�ce z domy�lnymi skr�tami
   najpopularniejszych manager�w okien - daj mi zna�.

Q: Co jest do zrobienia, jakie s� znane b��dy?
A: Program jest doskona�y, nie ma b��d�w. Co mo�na by�oby jeszcze dopisa�,
   znajdziesz w pliku TODO.

Q: Co na tacce robi ryba?
A: Kolejne wersje Kydpdict b�d� wyposa�one nie tylko w ryb� na tacce ale i
   plastikowe sztu�ce. Na serio: nie s�ysza�e� o babelfish? Id� do biblioteki,
   ksi�garni i poszukaj ksi��ek Douglasa Adamsa. Albo wybierz si� do kina.

Q: A co ze s�ownikiem PWN Oxford 2004 w kartonowym pude�ku za dwie ba�ki?
A: Podstawowy s�ownik ju� dzia�a. Co do sampli z wymow�, to nie mam danych do
   testowania. Podobnie z tabelami i dodatkowymi informacjami o gramatyce - niby
   s�, ale nie bardzo wiem co z nimi zrobi� w obecnej formie programu.
   
Q: Jak wyrazi� wdzi�czno�� i zmotywowa� do dalszej pracy?
   W dobie bankowo�ci elektronicznej to proste - chcesz zaokr�gli� stan rachunku w
   d� :)? Jest mo�liwo��; oto numer mojego konta: 60 1140 2004 0000 3202 3234 6486
```


#6. KONTAKT
Prosz� pisa� w sprawie programu na Berdy^W ytm@elysium.pl, sprawami zwi�zanymi
z modu�em YDP mo�e by� r�wnie� zainteresowany Andrzej Para <kulozik@op.pl>.
