/* Terminal colors (16 first used in escape sequence) */
 const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#222222", /* black   */
  [1] = "#e84f4f", /* red     */
  [2] = "#b7ce42", /* green   */
  [3] = "#fea63c", /* yellow  */
  [4] = "#66aabb", /* blue    */
  [5] = "#b7416e", /* magenta */
  [6] = "#6d878d", /* cyan    */
  [7] = "#dddddd", /* white   */

  /* 8 bright colors */
  [8]  = "#666666", /* black   */
  [9]  = "#d23d3d", /* red     */
  [10] = "#bde077", /* green   */
  [11] = "#ffe863", /* yellow  */
  [12] = "#aaccbb", /* blue    */
  [13] = "#e16a98", /* magenta */
  [14] = "#42717b", /* cyan    */
  [15] = "#cccccc", /* white   */

  /* special colors */
  [256] = "#161616", /* background */
  [257] = "#ffffff", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
 unsigned int defaultfg = 257;
 unsigned int defaultbg = 256;
 unsigned int defaultcs = 257;
