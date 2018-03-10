/* Terminal colors (16 first used in escape sequence) */
 const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#032c36", /* black   */
  [1] = "#c2454e", /* red     */
  [2] = "#7cbf9e", /* green   */
  [3] = "#8a7a63", /* yellow  */
  [4] = "#2e3340", /* blue    */
  [5] = "#ff5879", /* magenta */
  [6] = "#44b5b1", /* cyan    */
  [7] = "#f2f1b9", /* white   */

  /* 8 bright colors */
  [8]  = "#065f73", /* black   */
  [9]  = "#ef5847", /* red     */
  [10] = "#a2d9b1", /* green   */
  [11] = "#beb090", /* yellow  */
  [12] = "#61778d", /* blue    */
  [13] = "#ff99a1", /* magenta */
  [14] = "#9ed9d8", /* cyan    */
  [15] = "#f6f6c9", /* white   */

  /* special colors */
  [256] = "#021b21", /* background */
  [257] = "#e8dfd6", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
 unsigned int defaultfg = 257;
 unsigned int defaultbg = 256;
 unsigned int defaultcs = 257;
