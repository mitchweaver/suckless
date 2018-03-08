
/* Terminal colors (16 first used in escape sequence) */
const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#130f0c", /* black   */
  [1] = "#604c38", /* red     */
  [2] = "#8d7f85", /* green   */
  [3] = "#907256", /* yellow  */
  [4] = "#aa8d70", /* blue    */
  [5] = "#b9a38d", /* magenta */
  [6] = "#c4bfbf", /* cyan    */
  [7] = "#c4ae99", /* white   */

  /* 8 bright colors */
  [8]  = "#c69f77", /* black   */
  [9]  = "#cab9a8", /* red     */
  [10] = "#d5c6b8", /* green   */
  [11] = "#dcd1c7", /* yellow  */
  [12] = "#e0e0e2", /* blue    */
  [13] = "#e4dad2", /* magenta */
  [14] = "#eae3dc", /* cyan    */
  [15] = "#f6f2f0", /* white   */

  /* special colors */
  [256] = "#170f0d", /* background */
  [257] = "#f9f9f9", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
