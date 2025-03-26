class ZBossGaugeEffect : public ZScreenEffect
{
    public:
        virtual bool Draw(unsigned long int nTime, MDrawContext* pDC);
};

class ZScreenEffectManager : public ZEffectList {
    public:
        void Draw(MDrawContext* pDC);
};