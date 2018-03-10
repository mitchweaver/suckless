
/* Terminal colors (16 first used in escape sequence) */
 const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#2b303b", /* black   */
  [1] = "#bf616a", /* red     */
  [2] = "#a3be8c", /* green   */
  [3] = "#ebcb8b", /* yellow  */
  [4] = "#8fa1b3", /* blue    */
  [5] = "#b48ead", /* magenta */
  [6] = "#96b5b4", /* cyan    */
  [7] = "#c0c5ce", /* white   */

  /* 8 bright colors */
  [8]  = "#65737e", /* black   */
  [9]  = "#bf616a", /* red     */
  [10] = "#a3be8c", /* green   */
  [11] = "#ebcb8b", /* yellow  */
  [12] = "#8fa1b3", /* blue    */
  [13] = "#b48ead", /* magenta */
  [14] = "#96b5b4", /* cyan    */
  [15] = "#eff1f5", /* white   */

  /* special colors */
  [256] = "#2b303b", /* background */
  [257] = "#c0c5ce", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
 unsigned int defaultfg = 257;
 unsigned int defaultbg = 256;
 unsigned int defaultcs = 257;

