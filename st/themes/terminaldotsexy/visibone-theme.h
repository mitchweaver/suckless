/* Terminal colors (16 first used in escape sequence) */
 const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#666666", /* black   */
  [1] = "#cc6699", /* red     */
  [2] = "#99cc66", /* green   */
  [3] = "#cc9966", /* yellow  */
  [4] = "#6699cc", /* blue    */
  [5] = "#9966cc", /* magenta */
  [6] = "#66cc99", /* cyan    */
  [7] = "#cccccc", /* white   */

  /* 8 bright colors */
  [8]  = "#999999", /* black   */
  [9]  = "#ff99cc", /* red     */
  [10] = "#ccff99", /* green   */
  [11] = "#ffcc99", /* yellow  */
  [12] = "#99ccff", /* blue    */
  [13] = "#cc99ff", /* magenta */
  [14] = "#99ffcc", /* cyan    */
  [15] = "#ffffff", /* white   */

  /* special colors */
  [256] = "#333333", /* background */
  [257] = "#cccccc", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
 unsigned int defaultfg = 257;
 unsigned int defaultbg = 256;
 unsigned int defaultcs = 257;
