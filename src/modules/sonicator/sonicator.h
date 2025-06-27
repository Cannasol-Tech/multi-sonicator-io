class Sonicator {
public:

    int ctrlPin;
    int overloadErrPin;

    Sonicator();
    ~Sonicator();




    void start();
    void stop();

    bool isRunning() const;
    void setFrequency(int frequency);
    
    int getFrequency() const;
    void setAmplitude(int amplitude);
    int getAmplitude() const;   


};