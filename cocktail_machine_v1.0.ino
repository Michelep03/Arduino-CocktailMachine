#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

int pump1 = 2;  //pin digitale 2 a cui è collegata la pompa N1  //RUM
int pump2 = 3;  //pin digitale 3 a cui è collegata la pompa N2  //GIN
int pump3 = 4;  //pin digitale 4 a cui è collegata la pompa N3  //VODKA

int buzz = 10;  //pin digitale 10 a cui è collegato il buzzer

int pulsanteSX = 11;  //pin digitale 11 a cui è collegato il pulsante sinistro
int pulsanteDX = 12;  //pin digitale 12 a cui è collegato il pulsante destro
int pulsanteOK = 13;  //pin digitale 13 a cui è collegato il pulsante di selezione
//dichiarazione delle variabili in cui verrà memorizzato il valore della digitalRead (0 non premuto, 1 premuto)
int valDX, valOK, valSX;

//inizializzazione delle variabili in cui verrà memorizzato lo stato precedente del pulsante
int valDXOld = 0, valOKOld = 0, valSXOld = 0;  // per impedire che vada in uno scroll veloce

//inizializzazione della libreria in cui è descritta la modalità di utilizzo dei pin
LiquidCrystal_I2C lcd(0x27, 16, 2);  // impostazione dell'indirizzo dell'LCD 0x27 di 16 caratteri e 2 linee

//inizializzazione delle variabili per il multithreading
const int RSdelay = 10000;        //ReStart delay, tempo da trascorrere prima di resettare la selezione (in ms)
unsigned long RStime = millis();  //ReStart time, inizializzato al tempo trascorso dall'avvio del programma (in ms)

struct Dose {
  int numPompa;
  int quantita;
  char* name;
};

struct Cocktail {
  char* name;
  Dose ingredienti[3];
};

const int nCocktail = 6;  //numero dei cocktail memorizzati
int puntaCocktail = -1;   //indica il numero del cocktail selezionato, inizializzato a -1 perchè nessuno è selezionato

Cocktail Cocktails[nCocktail] = {
  {                              //copiare da qui
    .name = "Tequila Sun Rise",  //nome del cocktail

    { //lista di ingredienti
      {
        pump1,  //succo d'arancia
        180,    //180ml
        "Succo d'arancia" },
      { pump2,  //sciroppo di granatina
        30,     //30ml
        "Sciroppo di granatina" },
      { pump3,                   //tequila bianca
        90,                      //90ml
        "Tequila bianca" } } },  //a qui, per inserire un nuovo cocktail

  //-----------------------------
  {
    "Mojito",

    { //lista di ingredienti
      {
        .numPompa = pump1,  //Rum bianco
        .quantita = 45,     //45ml
        "Rum bianco" },
      { .numPompa = pump2,  //Succo di lime fresco
        .quantita = 20,     //20ml
        "Succo di lime" },
      { .numPompa = pump3,  //Soda
        .quantita = 40,     //40ml
        "Soda" } } },
  //-----------------------------
  {
    "Americano",

    { //lista di ingredienti
      {
        .numPompa = pump1,  //Campari
        .quantita = 30,     //30ml
        "Campari" },
      { .numPompa = pump2,  //Soda
        .quantita = 10,     // ? ml
        "Soda" },
      { .numPompa = pump3,  //Vermouth rosso
        .quantita = 30,     //30ml
        "Vermouth rosso" } } },
  //-----------------------------
  {
    "Moscow Mule",

    { //lista di ingredienti
      {
        .numPompa = pump1,  //Vodka
        .quantita = 50,     //50ml
        "Vodka" },
      { .numPompa = pump2,  //Ginger beer
        .quantita = 120,    //120ml
        "Ginger beer" } } },
  //-----------------------------
  {
    "Gin Tonic",

    { //lista di ingredienti
      {
        .numPompa = pump1,  //Gin
        .quantita = 60,    //60ml
        "Gin"
      },
      {
        .numPompa = pump2,  //Acqua tonica
        .quantita = 110,     //110ml
        "Acqua tonica"
      } } },
  //-----------------------------
  {
    "Aperol Spritz",

    { //lista di ingredienti
      {
        .numPompa = pump1,  //Aperol
        .quantita = 60,     //60ml
        "Aperol"
      },
      {
        .numPompa = pump2,  //Prosecco
        .quantita = 90,     //90ml
        "Prosecco"
      },
      {
        .numPompa = pump3,  //Soda
        .quantita = 10,     // ? ml
        "Soda"
      } } },

};  //fine array cocktail

//#################################################
void setup() {
  lcd.begin();      //inizializzazione dell'LCD
  lcd.backlight();  //attivazione della retroilluminazione

  //imposta i pin a cui sono collegate le pompe ad OUTPUT
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(pump3, OUTPUT);

  pinMode(buzz, OUTPUT);  //imposta il pin collegato al buzzer come OUTPUT

  //imposta i pin a cui sono collegati i pulsanti ad INPUT
  pinMode(pulsanteDX, INPUT);
  pinMode(pulsanteOK, INPUT);
  pinMode(pulsanteSX, INPUT);

  avvio();
}
//####################################################
void loop() {
  if (puntaCocktail == -1)  //se non è selezionato nessun cocktail
  {
    digitalWrite(pump1, HIGH);
    digitalWrite(pump2, HIGH);
    digitalWrite(pump3, HIGH);
  }

  valDX = digitalRead(pulsanteDX);  //lettura dell'input (pulsante) e memorizzazione in valDX
  valOK = digitalRead(pulsanteOK);  //lettura dell'input (pulsante) e memorizzazione in valOK
  valSX = digitalRead(pulsanteSX);  //lettura dell'input (pulsante) e memorizzazione in valSX

  //viene controllato se l'input sia HIGH (pulsante premuto) e prima prima fosse LOW (non premuto)
  if ((valDX == HIGH) && (valDXOld == LOW)) {
    buzzer();                                              //il buzzer riproduce un suono al click del pulsante
    puntaCocktail++;                                       //incrementa il contatore dei cocktail
    if (puntaCocktail > nCocktail - 1) puntaCocktail = 0;  //se è maggiore del numero dei cocktail, torna al primo
    cambiococktail();
    delay(250);  //antirimbalzo, si attende che l'input si stabilizzi
  }
  valDXOld = valDX;  //imposta lo stato precedente del pulsande destro al valore attuale

  if ((valSX == HIGH) && (valSXOld == LOW)) {
    buzzer();                                              //il buzzer riproduce un suono al click del pulsante
    puntaCocktail--;                                       //decrementa il contatore dei cocktail
    if (puntaCocktail < 0) puntaCocktail = nCocktail - 1;  //se è minore di 0, torna all'ultimo
    cambiococktail();
    delay(250);  //antirimbalzo, si attende che l'input si stabilizzi
  }
  valSXOld = valSX;  //imposta lo stato precedente del pulsande sinistro al valore attuale

  if ((valOK == HIGH) && (valOKOld == LOW)) {
    if (puntaCocktail == -1) {
      digitalWrite(buzz, HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
      delay(60);                 //aspetta
      digitalWrite(buzz, LOW);   //imposta a LOW (non passa corrente) il pin del buzzer
      delay(40);                 //aspetta
      digitalWrite(buzz, HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
      delay(60);                 //aspetta
      digitalWrite(buzz, LOW);   //imposta a LOW (non passa corrente) il pin del buzzer
      delay(40);                 //aspetta
      digitalWrite(buzz, HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
      delay(60);                 //aspetta
      digitalWrite(buzz, LOW);   //imposta a LOW (non passa corrente) il pin del buzzer

      lcd.clear();                   //pulisce lo schermo
      lcd.setCursor(0, 0);           //posiziona cursore in colonna 0 e riga 0
      lcd.print("Prima seleziona");  //stampa del testo su display
      lcd.setCursor(0, 1);           //posiziona cursore in colonna 0 e riga 1
      lcd.print("il cocktail");      //stampa del testo su display
      delay(4000);                   //aspetta 4 secondi
      avvio();
    } else {
      //riproduce il suono per 350ms
      digitalWrite(buzz, HIGH);
      delay(350);
      digitalWrite(buzz, LOW);

      lcd.setCursor(0, 0);            //posiziona cursore in colonna 0 e riga 0
      lcd.print("Sto preparando: ");  //stampa del testo su display
      prepara(puntaCocktail);
      delay(250);  //antirimbalzo, si attende che l'input si stabilizzi
    }
  }
  valOKOld = valOK;
  if (puntaCocktail != -1) restart();
}
//#########################################################
void buzzer() {
  //riproduce il suono per 150ms
  digitalWrite(buzz, HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
  delay(150);                //aspetta
  digitalWrite(buzz, LOW);   //imposta a LOW (non passa corrente) il pin del buzzer
}
void pulisciriga1() {
  //pulisce la riga 1
  lcd.setCursor(0, 1);            //posiziona cursore in colonna 0 e riga 1
  lcd.print("                ");  //stampa 16 spazi (riga vuota)
}
void cambiococktail() {
  RStime = millis();  //aggiorna il tempo passato dall'avvio del programma nella sua variabile
  pulisciriga1();
  lcd.setCursor(0, 1);                       //posiziona cursore in colonna 0 e riga 1
  lcd.print(Cocktails[puntaCocktail].name);  //stampa testo dell'array in posizione selezionata
}
void restart() {
  if ((millis() - RStime) > RSdelay) {  //se è passato più tempo del
    puntaCocktail = -1;                 //resetta la selezione del cocktail
    avvio();
  }
}
void avvio() {
  //testo iniziale
  lcd.setCursor(0, 0);            //posiziona cursore in colonna 0 e riga 0
  lcd.print("COCKTAIL MACHINE");  //stampa testo su display
  lcd.setCursor(0, 1);            //posiziona cursore in colonna 0 e riga 1
  lcd.print("Selezionane uno!");  //stampa testo su display
}
void prepara(int _cocktail) {

  //in 1.3 secondi sono 2 ml
  //in 13 secondi, sono 20 ml
  //ogni 10 ml sono 3 sec in piu

  for (int i = 0; i < 3; i++) {
    //while (Cocktails[_cocktail].ingredienti[i + 1].numPompa != NULL) {
    //foreach (Dose ingrediente in Cocktails[_cocktail].ingredienti) {
    if (Cocktails[_cocktail].ingredienti[i].numPompa != NULL) {

      lcd.clear();
      lcd.setCursor(0, 0);  //posiziona cursore in colonna 0 e riga 1
      //lcd.print("Erogo ");
      lcd.print("In ");

      double sec = convertiMl(Cocktails[_cocktail].ingredienti[i].quantita);
      lcd.print((int)(sec / 1000));  //stampa testo su display
      lcd.print("s ");

      lcd.print(Cocktails[_cocktail].ingredienti[i].quantita);
      lcd.print("ml di");

      lcd.setCursor(0, 1);  //posiziona cursore in colonna 0 e riga 1
      lcd.print(Cocktails[_cocktail].ingredienti[i].name);


      digitalWrite(Cocktails[_cocktail].ingredienti[i].numPompa, LOW);   //disattiva la prima pompa  //2ml
      delay(convertiMl(Cocktails[_cocktail].ingredienti[i].quantita));   //eroga per 2 secondi (1sec=1ml)
      digitalWrite(Cocktails[_cocktail].ingredienti[i].numPompa, HIGH);  //attiva la prima pompa (RUM)
    }
  }

  puntaCocktail = -1;  //resettiamo la selezione del cocktail con: nessuno selezionato
  delay(3000);         //dopo che ha finito, aspetta 3 secondi e stampa

  lcd.clear();
  lcd.setCursor(0, 0);            //posiziona cursore in colonna 0 e riga 0
  lcd.print("COCKTAIL PRONTO!");  //stampa testo su display
  lcd.setCursor(0, 1);            //posiziona cursore in colonna 0 e riga 1
  lcd.print("    Goditelo!");     //stampa testo su display

  delay(3000);
  avvio();
}

double convertiMl(double _ml) {
  return ((double)(_ml / 2) * 1300);
}