enum {
    NUM_SOCKET = 1,
    NUM_PHYSICAL_CPU_PER_SOCKET = 4,
    SMT_LEVEL = 2,
};

const int OS_CPU_ID[NUM_SOCKET][NUM_PHYSICAL_CPU_PER_SOCKET][SMT_LEVEL] = {
    { /* socket id: 0 */
        { /* physical cpu id: 0 */
          0, 1,     },
        { /* physical cpu id: 1 */
          2, 3,     },
        { /* physical cpu id: 2 */
          4, 5,     },
        { /* physical cpu id: 3 */
          6, 7,     },
    },
};
