/*
#define INPUT_TIP  {"", "adj", "adv", "conj", "perf", "m(f", "fus", "inv", "pron","nt", "npl", "cpd", "pl", "vb", "vi", "vt", "sg", \
	"abbr", "nom","acc", "dat", "gen", "infin", "instr", "loc", "irreg", "prep", "aux", "pt", "pp", "m", "n", "f", "post", "nvir", \
	"vir", "num", "decl", "excl", "inf!", "inf", "pej", "cmp", "pot!", "pot", "vr", "attr", "part", "fml", "im)perf", "dimin","ADMIN", "ANAT", "AUT", \
	"AVIAT", "BIO", "BOT", "BRIT", "CHEM", "COMM", "COMPUT", "CULIN", "ELEC", "FIN", "JUR", "LING", "MATH", "MED", "MIL", \
	"MUS", "NAUT", "POL", "PSYCH", "RAIL", "REL", "SCOL", "SPORT", "TECH", "TEL", "THEAT", "TYP", "US", "ZOOL", "fig", "lit", \
	"GEOG", "ARCHIT", "FIZ", "PHYSIOL", "imp", "GEOL", "art", "indef", "def", "PHOT", "ELEKTR", "EKON", "ECON", "GEOM", "JÊZ", \
	"KULIN", "KOMPUT", "LOT", "MAT", "MOT", "MUZ", "SZKOL", "WOJSK", "¯EGL", "BUD", "METEO", "HIST", "DRUK", "ROL", "pref", \
	"ASTRON", "PHYS", "etc", "AGR", "CONSTR", "suff", "LIT", "UNIV", "ups!"}


#define  OUTPUT_TIP  {"", "adjective - przymiotnik", "adverb - przys³ówek", "conjunction - spójnik", \
	"perfective verb - czasownik dokonany", "masculine(feminine) - mêski(¿eñski)", "inseparable verb - czasownik nierozdzielny", \
	"invariable - niezmienny", "pronoun - zaimek", "neuter - nijaki", "plural noun - rzeczownik w liczbie mnogiej", \
	"compound - wyraz z³o¿ony", "plural - liczba mnoga", "verb - czasownik", "intransitive verb - czasownik nieprzechodni", \
	"transitive verb - czasownik przechodni", "singular - liczba pojedyncza", \
	"abbreviation - skrót", "nominative - mianownik", "accusative - biernik", "dative - celownik", \
	"genitive - dope³niacz", "infinitive - bezokolicznik", "instrumental - narzêdnik", "locative - miejscownik", \
	"irregular - nieregularny", "preposition - przyimek", "auxiliary - pomocniczy", "past tense - czas przesz³y", \
	"past participle - imies³ów czasu przesz³ego", "masculine - mêski", "noun - rzeczownik", "feminine - ¿eñski", \
	"postpositiv (does not immediately precede a noun) - nie wystêpuje bezpo¶rednio przed rzeczownikiem", "nonvirile - niemêskoosobowy", \
	"virile - mêskoosobowy", "number - liczba", "declined - odmienny", "exclamation - wykrzyknik", \
	"offensiv - obra¼liwy, wulgarny", "informal - potocznie", "pejorative - pejoratywny", "compaund - wyraz z³o¿ony", \
	"obra¼liwy - offensiv", "potoczny - informal", "reflexiv verb - czasownik zwrotny", "attribute - przydawka", \
	"particle - partyku³a", "formal - formalny", "imperfect tense - czas przesz³y o aspekcie niedokonanym", \
	"diminutive - zdrobnienie", "administration - administracja", \
	"anatomy - anatomia", "automobiles - motoryzacja", "aviation - lotnictwo",  "biology - biologia", \
	"botany - botanika", "British English - angielszczyzna brytyjska", "chemistry - chemia", "commerce - handel", \
	"computer - informatyka i komputery", "culinary - kulinarny", "electronics, eletricity - elektronika, elektryczno¶æ", \
	"finance finanse", "law - prawo", "linguistic - jêzykoznawstwo", "mathematics - matematyka", \
	"medicine - medycyna", "military - wojskowo¶æ", "music - muzyka", "nautical - ¿egluga", "politics - polityka", \
	"psychology - psychologia", "railways - kolej", "religion - religia", "school - szko³a", "sport - sportowy", \
	"technology - technika i technologia", "telecominication - telekomunikacja", "theatre - teatr", "printing - poligrafia", \
	"American English - angielszczyzna amerykañska", "zoology - zoologia", "figurative - przeno¶ny", "literal - dos³owny", \
	"geography - geografia", "architecture - architektura", "fizyka - physics", "physiology - fizjologia", \
	"imperative - tryb rozkazuj±cy", "geology - geologia","article - rodzajnik", "indefinite - nieokre¶lony", \
	"definite - okre¶lony", "photography - fotografia", "elektryczno¶æ - electricity", "ekonomia - economy", \
	"economy - ekonomia", "geometria - geometry", "jêzykoznawstwo - linguistic", "kulinarny - culinary", \
	"komputery - computers", "lotnictwo - aviation", "matematyka - mathematics", "motoryzacja - automobiles", \
	"muzyka - music", "szko³a - school", "wojskowo¶æ - military", "¿egluga - nautical", "budownictwo - construction", \
	"meteorology - meteorologia", "history - historia", "poligrafia - printing", "rolnictwo - agriculture", \
	"prefix - przedrostek", "astronomy - astronomia", "physics - fizyka", "et cetera - itd.", "agriculture - rolnictwo", \
	"construction - budownictwo", "ups!"}
*/
	
#define INPUT_TIP  {"", "abbr","acc", "adj", "adv", "art", "attr", "aux", "cmp", "conj", "cpd", "dat", "decl", "dimin", "etc", \
	"excl", "f", "fig", "fml", "fus", "gen", "imp", "im)perf", "inf", "inf!", "indef", "infin", "instr", "inv", "irreg", "lit", "loc", "m", "m(f", \
	"n", "nom", "n(pl", "npl", "nt", "num", "nvir", "part", "pej", "perf", "post", "pot", "pot!", "pref", "prep", "pron", "pl", "pp", "pt", "sg", "suff", \
	"vb", "vi","vir", "vr", "vt","ADMIN", "AGR", "ANAT", "ARCHIT", "ASTRON", "AUT", "AVIAT", "BIO", "BOT", "BRIT",  "BUD", "CHEM", "COMM", \
	"COMPUT", "CONSTR", "CULIN", "DRUK", "ECON", "EKON", "ELEC", "ELEKTR", "FOT",  "FIN", \
	"FIZ", "GEOG", "GEOL", "GEOM", "HIST",  "JÊZ", "JUR", "KOMPUT","KULIN",  "LING",  "LIT", "LOT", "MAT", \
	"MATH", "MED", "METEO",  "MIL", "MOT", "MUS", "MUZ", "NAUT", "PHOT", "PHYSIOL", "POL", "PSYCH", "RAIL", \
	"REL", "ROL", "SCOL", "SPORT", "SZKOL", "TECH", "TEL", "THEAT", "TYP", "UNIV", "US", "WOJSK", "ZOOL", "¯EGL", "ups!"}

#define  OUTPUT_TIP  {"", "abbreviation - skrót", "accusative - biernik", "adjective - przymiotnik", "adverb - przys³ówek", \
	"article - rodzajnik", "attribute - przydawka", "auxiliary - pomocniczy", "compound - wyraz z³o¿ony", "conjunction - spójnik", \
	"compound - wyraz z³o¿ony", "dative - celownik", "declined - odmienny","diminutive - zdrobnienie", "et cetera - itd.", \
	"exclamation - wykrzyknik", "feminine - ¿eñski", "figurative - przeno¶ny", "formal - formalny", "inseparable verb - czasownik nierozdzielny", \
	"genitive - dope³niacz", "imperative - tryb rozkazuj±cy", "imperfect tense - czas przesz³y o aspekcie niedokonanym", \
	"informal - potocznie","offensiv - obra¼liwy, wulgarny", "indefinite - nieokre¶lony", "infinitive - bezokolicznik", "instrumental - narzêdnik", "invariable - niezmienny", \
	"irregular - nieregularny", "literal - dos³owny", "locative - miejscownik", "masculine - mêski", "masculine(feminine) - mêski(¿eñski)", \
	"noun - rzeczownik", "nominative - mianownik", "?", "plural noun - rzeczownik w liczbie mnogiej", "neuter - nijaki", "number - liczba", \
	"nonvirile - niemêskoosobowy", "particle - partyku³a", "pejorative - pejoratywny","perfective verb - czasownik dokonany", \
	"postpositiv (does not immediately precede a noun) - nie wystêpuje bezpo¶rednio przed rzeczownikiem",  "informal - potocznie", \
	"obra¼liwy - offensiv", "prefix - przedrostek", "preposition - przyimek", "pronoun - zaimek", "plural - liczba mnoga", \
	"past participle - imies³ów czasu przesz³ego", "past tense - czas przesz³y", "singular - liczba pojedyncza", "suffix - przyrostek", \
	"verb - czasownik", "intransitive verb - czasownik nieprzechodni", "virile - mêskoosobowy", "reflexiv verb - czasownik zwrotny", \
	"transitive verb - czasownik przechodni", "administration - administracja", "agriculture - rolnictwo", "anatomy - anatomia", \
	"architecture - architektura", "astronomy - astronomia", "automobiles - motoryzacja", "lotnictwo - aviation",  "biology - biologia", \
	"botany - botanika", "British English - angielszczyzna brytyjska", "budownictwo - construction", "chemistry - chemia", "commerce - handel", \
	"computer - informatyka i komputery","construction - budownictwo", "culinary - kulinarny",  "poligrafia - printing", "economy - ekonomia", "ekonomia - economy", \
	"electronics, eletricity - elektronika, elektryczno¶æ", "elektronika, elektryczno¶æ - electronics, eletricity","fotografia - photography", "finance - finanse", \
	"fizyka - physics", "geography - geografia", "geology - geologia", "geometria - geometry", "history - historia", "jêzykoznawstwo - linguistic", "law - prawo", \
	"komputery - computers", "kulinarny - culinary", "linguistic - jêzykoznawstwo", "literatura - literature", "lotnictwo - aviation", "matematyka - mathematics", \
	"mathematics - matematyka", "medicine - medycyna", "meteorology - meteorologia", "military - wojskowo¶æ", "motoryzacja - automobiles", "music - muzyka", \
	"muzyka - music", "nautical - ¿egluga", "photography - fotografia", "physiology - fizjologia", "politics - polityka", "psychology - psychologia", "railways - kolej", \
	"religion - religia", "rolnictwo - agriculture", "school - szko³a", "sport - sportowy", "szko³a - school", "technology - technika i technologia", \
	"telecommunication - telekomunikacja", "theatre - teatr", "printing - poligrafia",  "university - uniwersytet", "American English - angielszczyzna amerykañska", \
	"wojskowo¶æ - military", "zoology - zoologia", "¿egluga - nautical", "ups!"}

#define I_size 60
// that '-1' comes from "ups!" at the very end of the list
#define II_size 64-1
