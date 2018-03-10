const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#2f1e2e", /* black   */
  [1] = "#ef6155", /* red     */
  [2] = "#48b685", /* green   */
  [3] = "#fec418", /* yellow  */
  [4] = "#06b6ef", /* blue    */
  [5] = "#815ba4", /* magenta */
  [6] = "#5bc4bf", /* cyan    */
  [7] = "#a39e9b", /* white   */

  /* 8 bright colors */
  [8]  = "#776e71", /* black   */
  [9]  = "#ef6155", /* red     */
  [10] = "#48b685", /* green   */
  [11] = "#fec418", /* yellow  */
  [12] = "#06b6ef", /* blue    */
  [13] = "#815ba4", /* magenta */
  [14] = "#5bc4bf", /* cyan    */
  [15] = "#e7e9db", /* white   */

  /* special colors */
  [256] = "#2f1e2e", /* background */
  [257] = "#a39e9b", /* foreground */
};

unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
