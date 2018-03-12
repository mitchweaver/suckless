const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#101010", /* black   */
  [1] = "#7c7c7c", /* red     */
  [2] = "#8e8e8e", /* green   */
  [3] = "#a0a0a0", /* yellow  */
  [4] = "#686868", /* blue    */
  [5] = "#747474", /* magenta */
  [6] = "#868686", /* cyan    */
  [7] = "#b9b9b9", /* white   */

  /* 8 bright colors */
  [8]  = "#525252", /* black   */
  [9]  = "#7c7c7c", /* red     */
  [10] = "#8e8e8e", /* green   */
  [11] = "#a0a0a0", /* yellow  */
  [12] = "#686868", /* blue    */
  [13] = "#747474", /* magenta */
  [14] = "#868686", /* cyan    */
  [15] = "#111111", /* white   */

  /* special colors */
  [256] = "#f7f7f7", /* background */
  [257] = "#464646", /* foreground */
};

unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
