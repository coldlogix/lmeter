//----------------------------------------------------
//    Thomas Ortlepp    21.04.2004
//    Version 1.2
//    Lmeter Terminal Maker
//
//----------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

char ciffile[250],ciffile2[250];
char s[50],f6[50]=".cif",f7[50]=".cif2";   // cif-file suffix

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
char tz[50]="94",te[50]="TEXT;",tend[50]="E";   // Text entry
//---------------------------------------------
void textread()
  {
    int c,tn=0;
    printf("CIF File search for Text -> %s  \n------\n",ciffile);
    char row[20][50];
    FILE *ez; 
    ez=fopen(ciffile,"r+");
 
    do {
    fscanf(ez,"%s",s);
    while ((strcmp(s,tend)!=0)&&(strcmp(s,tz)!=0)) 
                 fscanf(ez,"%s",s);   // Text identifier found
    if (strcmp(s,tz)==0)
    {      
      c=0;
      do {
        fscanf(ez,"%s",s);  
        strcpy(row[c++],s);
     //   printf("%s ",s);
      } while (strcmp(s,te)!=0);
      if (c==6)
        { 
	   strcpy(tname[tn],row[0]);
	   strcpy(tdown[tn],row[1]);
   	   strcpy(ttop[tn],row[2]);	   
	   strcpy(tx[tn],row[3]);
	   txx[tn]=atoi(row[3]);
	   tyy[tn]=atoi(row[4]);	   
     	   printf("Term %d: %s  (%s,%s)",tn+1,row[0],row[2],row[1]);          
           printf("@(%s,%s)\n",row[3],row[4]);             	 	 

	   strcpy(ty[tn++],row[4]);	   		   
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
         assert("Text not inside a Terminal\n");
       }
//-----------------------------------------------------		
      fprintf(ey,"5 99 %s %s %s;\n",tname[k-1],tdown[k-1],ttop[k-1]);
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-	Line-Terminal       
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
       
    printf("\nLMeter Terminal Generator 1.2 --------------------------\n");
    printf("Thomas Ortlepp ------------------------------------------\n");
    printf("Last change: 21.04.2004----------------------------------\n");  
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
        printf("Usage: \n");      
        printf("ltm <cif-file>\n"); 
	exit(1);
      }	
        	    
    textread();
    termein();
    return 0;
  }




