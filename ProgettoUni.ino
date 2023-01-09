#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

int pump1=2;  //pin digitale 2 a cui è collegata la pompa N1  //RUM
int pump2=3;  //pin digitale 3 a cui è collegata la pompa N2  //GIN
int pump3=4;  //pin digitale 4 a cui è collegata la pompa N3  //VODKA
int pump4=5;  //pin digitale 5 a cui è collegata la pompa N4  //APEROL


int buzz=10; //pin digitale 10 a cui è collegato il buzzer

int pulsanteSX=11;  //pin digitale 11 a cui è collegato il pulsante sinistro
int pulsanteDX=12;  //pin digitale 12 a cui è collegato il pulsante destro
int pulsanteOK=13;  //pin digitale 13 a cui è collegato il pulsante di selezione
//dichiarazione delle variabili in cui verrà memorizzato il valore della digitalRead (0 non premuto, 1 premuto)
int valDX,valOK,valSX;

//inizializzazione delle variabili in cui verrà memorizzato lo stato precedente del pulsante
int valDXOld=0,valOKOld=0,valSXOld=0; // per impedire che vada in uno scroll veloce

const int nCocktail=5;  //numero dei cocktail memorizzati
char* cocktail[nCocktail]={"Mojito","Americano","Moscow Mule","Gin Tonic","Aperol Spritz"};  //array di stringhe
int puntaCocktail=-1; //indica il numero del cocktail selezionato, inizializzato a -1 perchè nessuno è selezionato

//inizializzazione della libreria in cui è descritta la modalità di utilizzo dei pin
LiquidCrystal_I2C lcd(0x27,16,2); // impostazione dell'indirizzo dell'LCD 0x27 di 16 caratteri e 2 linee

//inizializzazione delle variabili per il multithreading
const int RSdelay = 10000;   //ReStart delay, tempo da trascorrere prima di resettare la selezione (in ms)
unsigned long RStime = millis();  //ReStart time, inizializzato al tempo trascorso dall'avvio del programma (in ms)

struct Dose {
  int numPompa;
  int quantita;
};

struct Cocktail {
  char* name;  
  Dose ingredienti[];
};

/*Cocktail Cocktails[] = {
  { //da qui
    .name = "Tequila Sun Rise",

    .ingredienti[3] = {
      {
        .numPompa = pump1, //succo d'arancia
        .quantita = 180, //ml
      },
      {
        .numPompa = pump2,
        .quantita = 30, //ml
      },
      {
        .numPompa = pump3, //tequila
        .quantita = 78, //ml
      }
    }
  }, //a qui, per inserire un nuovo cocktail

  
};//fine array cocktail*/

//#################################################
void setup()
{
  lcd.begin();  //inizializzazione dell'LCD
  lcd.backlight();  //attivazione della retroilluminazione

  //imposta i pin a cui sono collegate le pompe ad OUTPUT
  pinMode(pump1,OUTPUT);
  pinMode(pump2,OUTPUT);
  pinMode(pump3,OUTPUT);
  pinMode(pump4,OUTPUT);

  pinMode(buzz,OUTPUT); //imposta il pin collegato al buzzer come OUTPUT

  //imposta i pin a cui sono collegati i pulsanti ad INPUT
  pinMode(pulsanteDX,INPUT);
  pinMode(pulsanteOK,INPUT);
  pinMode(pulsanteSX,INPUT);

  avvio(); 
}
//####################################################
void loop()
{

  if(puntaCocktail == -1) //se non è selezionato nessun cocktail
  {
    digitalWrite(pump1,HIGH); //attiva la prima pompa (RUM)
    digitalWrite(pump2,HIGH);  //disattiva la prima pompa  //2ml
    digitalWrite(pump3,HIGH);  //disattiva la prima pompa  //2ml
  }
  
  valDX=digitalRead(pulsanteDX);  //lettura dell'input (pulsante) e memorizzazione in valDX
  valOK=digitalRead(pulsanteOK);  //lettura dell'input (pulsante) e memorizzazione in valOK
  valSX=digitalRead(pulsanteSX);  //lettura dell'input (pulsante) e memorizzazione in valSX

  //viene controllato se l'input sia HIGH (pulsante premuto) e prima prima fosse LOW (non premuto)
  if((valDX==HIGH)&&(valDXOld==LOW)){
    buzzer(); //il buzzer riproduce un suono al click del pulsante
    puntaCocktail++;  //incrementa il contatore dei cocktail
    if(puntaCocktail>nCocktail-1) puntaCocktail=0;  //se è maggiore del numero dei cocktail, torna al primo
    cambiococktail();  
    delay(250); //antirimbalzo, si attende che l'input si stabilizzi
  }
  valDXOld=valDX; //imposta lo stato precedente del pulsande destro al valore attuale

  if((valSX==HIGH)&&(valSXOld==LOW)){
    buzzer(); //il buzzer riproduce un suono al click del pulsante
    puntaCocktail--;  //decrementa il contatore dei cocktail
    if(puntaCocktail<0) puntaCocktail=nCocktail-1;  //se è minore di 0, torna all'ultimo
    cambiococktail();
    delay(250); //antirimbalzo, si attende che l'input si stabilizzi
  }
  valSXOld=valSX; //imposta lo stato precedente del pulsande sinistro al valore attuale

  if((valOK==HIGH)&&(valOKOld==LOW)){
    if(puntaCocktail==-1){
      digitalWrite(buzz,HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
      delay(60); //aspetta
      digitalWrite(buzz,LOW); //imposta a LOW (non passa corrente) il pin del buzzer
      delay(40); //aspetta
      digitalWrite(buzz,HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
      delay(60); //aspetta
      digitalWrite(buzz,LOW); //imposta a LOW (non passa corrente) il pin del buzzer
      delay(40); //aspetta
      digitalWrite(buzz,HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
      delay(60); //aspetta
      digitalWrite(buzz,LOW); //imposta a LOW (non passa corrente) il pin del buzzer

      lcd.clear();  //pulisce lo schermo
      lcd.setCursor(0,0); //posiziona cursore in colonna 0 e riga 0
      lcd.print("Prima seleziona"); //stampa del testo su display
      lcd.setCursor(0,1); //posiziona cursore in colonna 0 e riga 1
      lcd.print("il cocktail"); //stampa del testo su display
      delay(4000);  //aspetta 4 secondi
      avvio();
    }
    else{
      //riproduce il suono per 350ms
      digitalWrite(buzz,HIGH);
      delay(350);
      digitalWrite(buzz,LOW);

      lcd.setCursor(0,0); //posiziona cursore in colonna 0 e riga 0
      lcd.print("Sto preparando: ");  //stampa del testo su display
      prepara(puntaCocktail);
      delay(250); //antirimbalzo, si attende che l'input si stabilizzi
    }
  }
  valOKOld=valOK;
  if(puntaCocktail!=-1) restart();
}
//#########################################################
void buzzer(){
  //riproduce il suono per 150ms
  digitalWrite(buzz,HIGH);  //imposta ad HIGH (passa corrente) il pin del buzzer
  delay(150); //aspetta
  digitalWrite(buzz,LOW); //imposta a LOW (non passa corrente) il pin del buzzer
}
void pulisciriga1(){
  //pulisce la riga 1
  lcd.setCursor(0,1); //posiziona cursore in colonna 0 e riga 1
  lcd.print("                ");  //stampa 16 spazi (riga vuota)
}
void cambiococktail(){
  RStime = millis();  //aggiorna il tempo passato dall'avvio del programma nella sua variabile
  pulisciriga1();
  lcd.setCursor(0,1); //posiziona cursore in colonna 0 e riga 1
  lcd.print(cocktail[puntaCocktail]); //stampa testo dell'array in posizione selezionata
}
void restart(){
  if( (millis() - RStime) > RSdelay ){  //se è passato più tempo del
    puntaCocktail=-1; //resetta la selezione del cocktail
    avvio();
  }
}
void avvio(){
  //testo iniziale
  lcd.setCursor(0,0); //posiziona cursore in colonna 0 e riga 0
  lcd.print("COCKTAIL MACHINE");  //stampa testo su display
  lcd.setCursor(0,1); //posiziona cursore in colonna 0 e riga 1
  lcd.print("Selezionane uno!"); //stampa testo su display
}
void prepara(int x){
  switch(x){
    case 0:
  //in 10 secondi una pompa riempie circa 20ml
  //ergo, 2ml al secondo

      digitalWrite(pump1,LOW);  //disattiva la prima pompa  //2ml
      delay(10000);  //eroga per 2 secondi (1sec=1ml)
      digitalWrite(pump1,HIGH); //attiva la prima pompa (RUM)
      
      delay(1000);
      digitalWrite(pump2,LOW);  //disattiva la prima pompa  //2ml
      delay(2000);  //eroga per 2 secondi (1sec=1ml)
      digitalWrite(pump2,HIGH); //attiva la prima pompa (RUM)
      
      delay(1000);
      digitalWrite(pump3,LOW);  //disattiva la prima pompa  //2ml
      delay(2000);  //eroga per 2 secondi (1sec=1ml)
      digitalWrite(pump3,HIGH); //attiva la prima pompa (RUM)
    break;
  }
  puntaCocktail=-1;
  delay(3000);  //dopo che ha finito, aspetta 3 secondi e stampa
  lcd.clear();
  lcd.setCursor(0,0); //posiziona cursore in colonna 0 e riga 0
  lcd.print("COCKTAIL PRONTO!");  //stampa testo su display
  lcd.setCursor(0,1); //posiziona cursore in colonna 0 e riga 1
  lcd.print("    Goditelo!"); //stampa testo su display
  delay(3000);
  avvio();
}

