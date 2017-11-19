//----------------------------------------------------
//    Thomas Ortlepp    21.04.2004
//    Version 1.2
//    Lmeter Terminal Maker
//
//----------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <fstream.h>
#include <string.h>
#include <assert.h>

char ciffile[250],ciffile2[250];
char s[50],f6[50]=".cif",f7[50]=".cif2";   // Endung cif-File
char lterms[50]="LTERM;",boxs[50]="B",newline[50]="\n";
//-------
char tname[64][50];
char tdown[64][50];
char ttop[64][50];
char tx[64][50],ty[64][50];
int  txx[64],tyy[64];
//--------
char tboxdx[64];
char tboxdy[64];
char tboxx[64];
char tboxy[64];

int TN;
//FILE *ez;

//------------------------------
char tz[50]="94",te[50]="TEXT;",tend[50]="E";   // Text Eintrag
//---------------------------------------------
void textread()
  {
    int c,tn=0;
    printf("CIF File nach Text durchsuchen -> %s  \n------\n",ciffile);
    char zeile[20][50];
    FILE *ez; 
    ez=fopen(ciffile,"r+");
 
    do {
    fscanf(ez,"%s",s);
    while ((strcmp(s,tend)!=0)&&(strcmp(s,tz)!=0)) 
                 fscanf(ez,"%s",s);   // Text Kennung gefunden      
    if (strcmp(s,tz)==0)
    {      
      c=0;
      do {
        fscanf(ez,"%s",s);  
        strcpy(zeile[c++],s);
     //   printf("%s ",s);
      } while (strcmp(s,te)!=0);
      if (c==6)
        { 
	   strcpy(tname[tn],zeile[0]);
	   strcpy(tdown[tn],zeile[1]);
   	   strcpy(ttop[tn],zeile[2]);	   
	   strcpy(tx[tn],zeile[3]);
	   txx[tn]=atoi(zeile[3]);
	   tyy[tn]=atoi(zeile[4]);	   
     	   printf("Term %d: %s  (%s,%s)",tn+1,zeile[0],zeile[2],zeile[1]);          
           printf("@(%s,%s)\n",zeile[3],zeile[4]);             	 	 

	   strcpy(ty[tn++],zeile[4]);	   		   
        }
     }
    TN=tn; 
    // printf("<%s>\n",s);
    }while(strcmp(s,tend)!=0);
   printf("-----\n");    
    fclose(ez);    
  }
//---------------------------------------------


void termein()
  {
    printf("CIF File Copy -> %s  \n------\n",ciffile2);
    FILE *ez,*ey; 
    char c=';';
    ez=fopen(ciffile,"r+");
    ey=fopen(ciffile2,"w+");    
    int i=0,z1,z2;
    int lterm=0,tmer;
    int Dx,Dy,Tx,Ty; 
    int k=0,found=0,TX,TY,DX,DY;
     
    while ( fscanf(ez,"%s\n",s)!=EOF)
     {
        tmer=0;
	if (s[0]=='L') lterm=0;
	if (strcmp(s,lterms)==0) lterm=1;
	
	if (lterm==1)
	  {
	    if (strcmp(s,boxs)==0)
	      {
                fscanf(ez,"%s\n",s);
                strcpy(tboxdx,s);    
                fscanf(ez,"%s\n",s);
                strcpy(tboxdy,s);   
                fscanf(ez,"%s\n",s);
                strcpy(tboxx,s);   
                fscanf(ez,"%s\n",s);
                strcpy(tboxy,s);   
                 
		DX=atoi(tboxdx);        
		DY=atoi(tboxdy);
		TX=atoi(tboxx);
		TY=atoi(tboxy);
				             
		i=0;  
		tmer=1;
//-----------------------------------------------------
     k=0;
     found=0;
//     printf("%d %d %d %d\n",DX,DY,TX,TY);
     do {
      if ((TX-DX/2<=txx[k])&&(TX+DX/2>=txx[k])&&
          (TY-DY/2<=tyy[k])&&(TY+DY/2>=tyy[k]))
	    {
              printf("Terminal-> %s %s %s %s <-%s\n",
                 tboxdx,tboxdy,tboxx,tboxy,tname[k]);

	      found=1;
	    }
      k++;       
     } while ((found==0)&&(k<TN));
     if (found==0)
       {
         assert("Text nicht innerhalb eines Terminals\n");
       }
//-----------------------------------------------------		
      fprintf(ey,"5 99 %s %s %s;\n",tname[k-1],tdown[k-1],ttop[k-1]);
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-	Linien-Terminal       
         if ((DX>0)&&(DY>0))
           fprintf(ey,"B %s %s %s %s ",tboxdx,tboxdy,tboxx,tboxy);  
	    else
		{
		  if (DX==0)
                   fprintf(ey,"W 0 %d %d %d %d;",TX,TY-DY/2,TX,TY+DY/2);         
		     else
		   fprintf(ey,"W 0 %d %d %d %d;",TX-DX/2,TY,TX+DX/2,TY);  
		}
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-	       		
		
	      }
	  }

        if (tmer==0) fprintf(ey,"%s ",s);  	i++;

        if ((s[strlen(s)-1]==';') || (i==13))
	  { 
	     fprintf(ey,"\n");     
	     i=0;
	  }
     }
 
 
    printf("------\n");    

    fclose(ez);    
    fclose(ey);
  }
//---------------------------------------------


  

int main (int argc, char *argv[])
  {
    int e,mode;
       
    printf("\nLMeter Terminal Generator 1.2 ------------------------\n");
    printf("Thomas Ortlepp ------------------------------------------\n");
    printf("letzte Aenderung: 21.04.2004-----------------------------\n");  
    printf("---------------------------------------------------------\n");      
    if (argc==2)
      {
	strcpy(ciffile,argv[1]);
	strcpy(ciffile2,argv[1]);	
 	strcat(ciffile,f6);
	strcat(ciffile2,f7);
      }	
    else    
      {
        printf("Programmaufruf: \n");      
        printf("ltm <cif-file>\n"); 
	exit(1);
      }	
        	    
    textread();
    termein();
    return 0;
  }




