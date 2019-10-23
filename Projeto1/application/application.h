struct applicationLayer
{
    int fileDescriptor; /* Descritor correspondente à porta série */
    int status; /* TRANSMITTER | RECEIVER */     
} applicationLayer;

struct applicationLayer application;