/* Terminal colors (16 first used in escape sequence) */
 const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#3b3228", /* black   */
  [1] = "#cb6077", /* red     */
  [2] = "#beb55b", /* green   */
  [3] = "#f4bc87", /* yellow  */
  [4] = "#8ab3b5", /* blue    */
  [5] = "#a89bb9", /* magenta */
  [6] = "#7bbda4", /* cyan    */
  [7] = "#d0c8c6", /* white   */

  /* 8 bright colors */
  [8]  = "#7e705a", /* black   */
  [9]  = "#cb6077", /* red     */
  [10] = "#beb55b", /* green   */
  [11] = "#f4bc87", /* yellow  */
  [12] = "#8ab3b5", /* blue    */
  [13] = "#a89bb9", /* magenta */
  [14] = "#7bbda4", /* cyan    */
  [15] = "#f5eeeb", /* white   */

  /* special colors */
  [256] = "#3b3228", /* background */
  [257] = "#d0c8c6", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
 unsigned int defaultfg = 257;
 unsigned int defaultbg = 256;
 unsigned int defaultcs = 257;
