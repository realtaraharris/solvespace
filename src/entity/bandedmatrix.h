class BandedMatrix {
public:
    enum {
        MAX_UNKNOWNS   = 16,
        RIGHT_OF_DIAG  = 1,
        LEFT_OF_DIAG   = 2
    };

    double A[MAX_UNKNOWNS][MAX_UNKNOWNS];
    double B[MAX_UNKNOWNS];
    double X[MAX_UNKNOWNS];
    int n;

    void Solve();
};