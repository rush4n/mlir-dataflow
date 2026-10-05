extern void consume(int);

int loop_counter(int limit) {
    int sum = 0;

    for (int i = 0; i < limit; ++i) {
        consume(i);
        sum += i;
    }

    return sum;
}

