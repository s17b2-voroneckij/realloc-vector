class Int {
private:
    int value;

public:
    static int moves;
    static int copies;
    static int constructed;

    Int() = delete;

    explicit Int(int a): value(a) {
        constructed++;
    }

    ~Int() {
        constructed--;
    }

    Int(const Int& other) {
        value = other.value;
        copies++;
        constructed++;
    }

    Int(Int&& other) noexcept {
        value = other.value;
        other.value = 0;
        moves++;
        constructed++;
    }

    Int& operator = (const Int& other) = delete;

    Int& operator = (Int&& other) = delete;
};