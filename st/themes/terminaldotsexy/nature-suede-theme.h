const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#392925", /* black   */
  [1] = "#98724c", /* red     */
  [2] = "#908f32", /* green   */
  [3] = "#aa964c", /* yellow  */
  [4] = "#7b854e", /* blue    */
  [5] = "#6b5644", /* magenta */
  [6] = "#5c5142", /* cyan    */
  [7] = "#c8b55b", /* white   */

  /* 8 bright colors */
  [8]  = "#544b2e", /* black   */
  [9]  = "#af652f", /* red     */
  [10] = "#c3c13d", /* green   */
  [11] = "#c8b55b", /* yellow  */
  [12] = "#70a16c", /* blue    */
  [13] = "#98724c", /* magenta */
  [14] = "#778725", /* cyan    */
  [15] = "#e4dc8c", /* white   */

  /* special colors */
  [256] = "#170f0d", /* background */
  [257] = "#746c48", /* foreground */
};

unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
