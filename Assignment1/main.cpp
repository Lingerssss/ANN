/************************************************************************
*  mlp.cpp - Implements a multi-layer back-propagation neural network

*  Ver1:JX Dove King - 25 Feb 2020


*************************************************************************/
#include<iostream>
#include<iomanip>
#include<fstream>
#include<cstdlib>
#include<cstdio>
#include<cmath>
#include<ctime>
#include<sstream>

using namespace std;

const int MAXN = 50;       // Max neurons in any layer
const int MAXPATS = 5000;  // Max training patterns

// mlp paramaters
long  NumIts ;    // Max training iterations
int NumHN;    // Number of hidden layers
int NumHN1;    // Number of neurons in hidden layer 1
int NumHN2;    // Number of neurons in hidden layer 2
int NumHN3;    // Number of neurons in hidden layer 3
int NumHN4;    // Number of neurons in hidden layer 4
float LrnRate;    // Learning rate
float Mtm1;    // Momentum(t-1)
float Mtm2;    // Momentum(t-2)
float ObjErr;    // Objective error

int CurrentHN;

// mlp weights
float **w1, **w11, **w111;// 1st layer wts
float **w2, **w22, **w222;// 2nd layer wts

double MinErr;
double MaxErr;
double AveErr;
double NumErr;
double PatErr;

void TrainNet(float **x, float **d, int NumIPs, int NumOPs, int NumPats);

void TrainNet3(float **x, float **d, int NumIPs, int NumOPs, int NumPats);

void TestNet(float **x, float **d, int NumIPs, int NumOPs, int NumPats);

float **Aloc2DAry(int m, int n);

void Free2DAry(float **Ary2D, int n);


int CHN;
float temp;

void Weight(int Numx, int Numy);

void County(float **x, float **temp, int Numx, int Numy, int NumPats);

void CountErr(float **d, float **temp, int NumIPs, int NumOPs, int NumPats);

void back(float **d, float **temp, int Numx, int Numy, int NumPats, int CHN);


string int_string(int i, string s);

int string_int(string s, int i);


float **hh = Aloc2DAry(NumHN, MAXN);
float **h1, **h2, **h3, **h4, **h5, **h6, **h7, **h8, **h9;


int NumTrnPats, NumTstPats, Ordering;


int main() {
    ifstream fin;
    int i, j, NumIPs, NumOPs, NumTrnPats, NumTstPats, Ordering;
    char Line[500], Tmp[20], FName[20];
    cout << "Enter data filename: ";
    cin >> FName;
    cin.ignore();
    fin.open("/Users/gongjiaxi/CLionProjects/Assignment1/data2.txt");
    if (!fin.good()) {
        cout << "File not found!\n";
        exit(1);
    }
    //read data specs...
    do{fin.getline(Line,500);}while(Line[0]==';'); //eat comments
    sscanf(Line,"%s%d",Tmp,&NumIPs);
    fin>>Tmp>>NumOPs;
    fin>>Tmp>>NumTrnPats;
    fin>>Tmp>>NumTstPats;
    fin>>Tmp>>NumIts;
    fin>>Tmp>>NumHN;
    i=NumHN;
    if(i-- > 0)fin>>Tmp>>NumHN1;
    if(i-- > 0)fin>>Tmp>>NumHN2;
    if(i-- > 0)fin>>Tmp>>NumHN3;
    if(i-- > 0)fin>>Tmp>>NumHN4;
    fin>>Tmp>>LrnRate;
    fin>>Tmp>>Mtm1;
    fin>>Tmp>>Mtm2;
    fin>>Tmp>>ObjErr;
    fin>>Tmp>>Ordering;
    if( NumIPs<1||NumIPs>MAXN||NumOPs<1||NumOPs>MAXN||
        NumTrnPats<1||NumTrnPats>MAXPATS||NumTrnPats<1||NumTrnPats>MAXPATS||
        NumIts<1||NumIts>20e6||NumHN1<0||NumHN1>50||
        LrnRate<0||LrnRate>1||Mtm1<0||Mtm1>10||Mtm2<0||Mtm2>10||ObjErr<0||ObjErr>10
            ){ cout<<"Invalid specs in data file!\n"; exit(1); }
    float **IPTrnData= Aloc2DAry(NumTrnPats,NumIPs);
    float **OPTrnData= Aloc2DAry(NumTrnPats,NumOPs);
    float **IPTstData= Aloc2DAry(NumTstPats,NumIPs);
    float **OPTstData= Aloc2DAry(NumTstPats,NumOPs);
    for(i=0;i<NumTrnPats;i++){
        for(j=0;j<NumIPs;j++)
            fin>>IPTrnData[i][j];
        for(j=0;j<NumOPs;j++)
            fin>>OPTrnData[i][j];
    }
    for(i=0;i<NumTstPats;i++){
        for(j=0;j<NumIPs;j++)
            fin>>IPTstData[i][j];
        for(j=0;j<NumOPs;j++)
            fin>>OPTstData[i][j];
    }
    fin.close();


    TrainNet3(IPTrnData,OPTrnData,NumIPs,NumOPs,NumTrnPats);
    //TrainNet(IPTrnData,OPTrnData,NumIPs,NumOPs,NumTrnPats);
    TestNet(IPTstData, OPTstData, NumIPs, NumOPs, NumTstPats);
    Free2DAry(IPTrnData, NumTrnPats);
    Free2DAry(OPTrnData,NumTrnPats);
    Free2DAry(IPTstData,NumTstPats);
    Free2DAry(OPTstData,NumTstPats);
    cout<<"End of program.\n";
    system("PAUSE");
    return 0;
}

 void  TrainNet(float **x,float **d,int NumIPs,int NumOPs,int NumPats ){
// Trains 2 layer back propagation neural network
// x[][]=>input data, d[][]=>desired output data

    float *h1 = new float[NumHN1]; // O/Ps of hidden layer
    float *y  = new float[NumOPs]; // O/P of Net
    float *ad1= new float[NumHN1]; // HN1 back prop errors
    float *ad2= new float[NumOPs]; // O/P back prop errors
    float PatErr,MinErr,AveErr,MaxErr;  // Pattern errors
    int p,i,j;     // for loops indexes
    long ItCnt=0;  // Iteration counter
    long NumErr=0; // Error counter (added for spiral problem)

    cout<<"TrainNet2: IP:"<<NumIPs<<" H1:"<<NumHN1<<" OP:"<<NumOPs<<endl;

    // Allocate memory for weights
    w1   = Aloc2DAry(NumIPs,NumHN1);// 1st layer wts
    w11  = Aloc2DAry(NumIPs,NumHN1);
    w111 = Aloc2DAry(NumIPs,NumHN1);
    w2   = Aloc2DAry(NumHN1,NumOPs);// 2nd layer wts
    w22  = Aloc2DAry(NumHN1,NumOPs);
    w222 = Aloc2DAry(NumHN1,NumOPs);

    // Init wts between -0.5 and +0.5
    srand(time(0));
    for(i=0;i<NumIPs;i++)
        for(j=0;j<NumHN1;j++)
            w1[i][j]=w11[i][j]=w111[i][j]= float(rand())/RAND_MAX - 0.5;
    for(i=0;i<NumHN1;i++)
        for(j=0;j<NumOPs;j++)
            w2[i][j]=w22[i][j]=w222[i][j]= float(rand())/RAND_MAX - 0.5;

    for(;;){// Main learning loop
        MinErr=3.4e38; AveErr=0; MaxErr=-3.4e38; NumErr=0;
        for(p=0;p<NumPats;p++){ // for each pattern...
            // Cal neural network output
            for(i=0;i<NumHN1;i++){ // Cal O/P of hidden layer 1
                float in=0;
                for(j=0;j<NumIPs;j++)
                    in+=w1[j][i]*x[p][j];
                h1[i]=(float)(1.0/(1.0+exp(float(-in))));// Sigmoid fn
            }
            for(i=0;i<NumOPs;i++){ // Cal O/P of output layer
                float in=0;
                for(j=0;j<NumHN1;j++)
                    in+=w2[j][i]*h1[j];

                y[i]=(float)(1.0/(1.0+exp(float(-in))));// Sigmoid fn
            }
            // Cal error for this pattern
            PatErr=0.0;
            for(i=0;i<NumOPs;i++){
                float err=y[i]-d[p][i]; // actual-desired O/P
                if(err>0)PatErr+=err; else PatErr-=err;
                NumErr += ((y[i]<0.5&&d[p][i]>=0.5)||(y[i]>=0.5&&d[p][i]<0.5));//added for binary classification problem
            }
            if(PatErr<MinErr)MinErr=PatErr;
            if(PatErr>MaxErr)MaxErr=PatErr;
            AveErr+=PatErr;

            // Learn pattern with back propagation
            for(i=0;i<NumOPs;i++){ // Modify layer 2 wts
                ad2[i]=(d[p][i]-y[i])*y[i]*(1.0-y[i]);
                for(j=0;j<NumHN1;j++){
                    w2[j][i]+=LrnRate*h1[j]*ad2[i]+
                              Mtm1*(w2[j][i]-w22[j][i])+
                              Mtm2*(w22[j][i]-w222[j][i]);
                    w222[j][i]=w22[j][i];
                    w22[j][i]=w2[j][i];
                }
            }
            for(i=0;i<NumHN1;i++){ // Modify layer 1 wts
                float err=0.0;
                for(j=0;j<NumOPs;j++)
                    err+=ad2[j]*w2[i][j];
                ad1[i]=err*h1[i]*(1.0-h1[i]);
                for(j=0;j<NumIPs;j++){
                    w1[j][i]+=LrnRate*x[p][j]*ad1[i]+
                              Mtm1*(w1[j][i]-w11[j][i])+
                              Mtm2*(w11[j][i]-w111[j][i]);
                    w111[j][i]=w11[j][i];
                    w11[j][i]=w1[j][i];
                }
            }
        }// end for each pattern
        ItCnt++;
        AveErr /= NumPats;
        float PcntErr = NumErr / float(NumPats) * 100.0;
        cout.setf(ios::fixed | ios::showpoint);
        cout << setprecision(6) << setw(6) << ItCnt << ": " << setw(12) << MinErr << setw(12) << AveErr << setw(12)
             << MaxErr << setw(12) << PcntErr << endl;//

        if ((AveErr <= ObjErr) || (ItCnt == NumIts)) break;
    }// end main learning loop
     // Free memory
     delete[] h1;
     delete[] y;
     delete[] ad1;
     delete[] ad2;

 }

void TrainNet3(float **x, float **d, int NumIPs, int NumOPs, int NumPats) {
    int ItCnt = 0;
    for (;;) {// Main learning loop
        MinErr = 3.4e38;
        AveErr = 0;
        MaxErr = -3.4e38;
        NumErr = 0;
        Aloc2DAry(NumHN, MAXN);// 1st layer wts
        Weight(NumIPs, NumHN1);
        Weight(NumHN1, NumOPs);
        County(x, h1, NumIPs, NumHN1, NumTrnPats);
        County(h1, h2, NumHN1, NumHN2, NumTrnPats);
        CountErr(d, h2, NumIPs, NumOPs, NumTrnPats);
        back(d, h2, NumIPs, NumOPs, NumTrnPats, 1);
        back(h2, h1, NumIPs, NumOPs, NumTrnPats, 0);
        ItCnt++;
        AveErr /= NumPats;
        float PcntErr = NumErr / float(NumPats) * 100.0;
        cout.setf(ios::fixed | ios::showpoint);
        cout << setprecision(6) << setw(6) << ItCnt << ": " << setw(12) << MinErr << setw(12) << AveErr << setw(12)
             << MaxErr << setw(12) << PcntErr << endl;//
        if ((AveErr <= ObjErr) || (ItCnt == NumIts)) break;
    }
    TrainNet(x, d, NumIPs, NumOPs, NumPats);
}

void Weight(int Numx,int Numy) {
    // Allocate memory for weights
    w1 = Aloc2DAry(Numx, Numy);// 1st layer wts
    w11 = Aloc2DAry(Numx, Numy);
    w111 = Aloc2DAry(Numx, Numy);
    // Init wts between -0.5 and +0.5
    int i, j;
    srand(time(0));
    for (i = 0; i < Numx; i++)
        for (j = 0; j < Numy; j++)
            w1[i][j] = w11[i][j] = w111[i][j] = float(rand()) / RAND_MAX - 0.5;
}

void County(float **x, float **temp, int Numx, int Numy, int NumPats) {
    int i, j, p;
    string s;
    temp = Aloc2DAry(NumPats, Numy);
    // for each pattern...
    // Cal neural network output
    for (p = 0; p < NumPats; p++) {
        for (i = 0; i < Numy; i++) { // Cal O/P of hidden layer 1
            float in = 0;
            for (j = 0; j < Numx; j++)
                in += w1[j][i] * x[p][j];
            temp[p][i] = (float) (1.0 / (1.0 + exp(float(-in))));// Sigmoid fn
        }
    }
}


// Cal error for this pattern
void CountErr(float **d, float **temp, int NumIPs, int NumOPs, int NumPats) {
    MinErr = 3.4e38;
    AveErr = 0;
    MaxErr = -3.4e38;
    NumErr = 0;
    PatErr = 0.0;
    int i, j, p;
    for (p = 0; p < NumPats; p++) {
        for (i = 0; i < NumOPs; i++) {
            float err = temp[NumHN - 1][i] - d[p][i]; // actual-desired O/P
            if (err > 0)PatErr += err; else PatErr -= err;
            NumErr += ((temp[NumHN - 1][i] < 0.5 && d[p][i] >= 0.5) ||
                       (temp[NumHN - 1][i] >= 0.5 && d[p][i] < 0.5));//added for binary classification problem
        }
        if (PatErr < MinErr)MinErr = PatErr;
        if (PatErr > MaxErr)MaxErr = PatErr;
        AveErr += PatErr;
    }

}


void back(float **d, float **temp, int Numx, int Numy, int NumPats, int CHN) {//CHN是隐藏层层数，这里hh[CHN+1][I]表示的是前置过程中算出的Y

    int i, j, p;

    for (p = 0; p < NumPats; p++) {
        float *ad = new float[Numy];
        for (i = 0; i < Numy; i++) { // Modify layer 2 wts
            ad[i] = (d[p][i] - temp[CHN + 1][i]) * temp[CHN + 1][i] * (1.0 - temp[CHN + 1][i]);
            for (j = 0; j < Numx; j++) {
                w1[j][i] += LrnRate * temp[CHN][j] * ad[i] +
                            Mtm1 * (w1[j][i] - w11[j][i]) +
                            Mtm2 * (w11[j][i] - w111[j][i]);
                w111[j][i] = w11[j][i];
                w11[j][i] = w1[j][i];
            }
        }
    }
}




void TestNet(float **x,float **d,int NumIPs,int NumOPs,int NumPats ) {
    int p, i, j;
    float *h1 = new float[NumHN1]; // O/Ps of hidden layer
    float *y = new float[NumOPs]; // O/P of Net
    float *ad1 = new float[NumHN1]; // HN1 back prop errors
    float *ad2 = new float[NumOPs]; // O/P back prop errors
    cout << "TestNet() not yet implemented\n";
    for (p = 0; p < NumPats; p++) { // for each pattern...
        // Cal neural network output
        for (i = 0;i < NumHN1;i++) { // Cal O/P of hidden layer 1
            float in = 0;
            for (j = 0; j < NumIPs; j++)
                in += *(*(w1 + j) + i) * x[p][j];
                h1[i] = (float) (1.0 / (1.0 + exp(float(-in))));// Sigmoid fn
            }
        for(i=0;i<NumOPs;i++){ // Cal O/P of output layer
            float in=0;
            for(j=0;j<NumHN1;j++)
                in+=*(*(w2+j)+i)*h1[j];
                y[i]=(float)(1.0/(1.0+exp(float(-in))));// Sigmoid fn
                cout<<setprecision(6)<<setw(6)<<h1[i]<<": "<<setw(12)<<y[i]<<endl;//
            }
    }
}
float **Aloc2DAry(int m,int n){
//Allocates memory for 2D array
    float **Ary2D = new float*[m];
    if(Ary2D==NULL){cout<<"No memory!\n";exit(1);}
    for(int i=0;i<m;i++){
        Ary2D[i] = new float[n];
        if(Ary2D[i]==NULL){cout<<"No memory!\n";exit(1);}
    }
    return Ary2D;
}

void Free2DAry(float **Ary2D, int n) {
//Frees memory in 2D array
    for (int i = 0; i < n; i++)
        delete[] Ary2D[i];
    delete[] Ary2D;
}


string int_string(int i) {
    stringstream stream;
    string s;
    stream << i;
    stream >> s;
    return s;

}

int string_int(string s) {
    stringstream stream;
    int i;
    stream << s;
    stream >> i;
    return i;
}
