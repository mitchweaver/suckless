
/* Terminal colors (16 first used in escape sequence) */
const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#353535", /* black   */
  [1] = "#744b40", /* red     */
  [2] = "#6d6137", /* green   */
  [3] = "#765636", /* yellow  */
  [4] = "#61564b", /* blue    */
  [5] = "#6b4a49", /* magenta */
  [6] = "#435861", /* cyan    */
  [7] = "#b3b3b3", /* white   */

  /* 8 bright colors */
  [8]  = "#5f5f5f", /* black   */
  [9]  = "#785850", /* red     */
  [10] = "#6f6749", /* green   */
  [11] = "#776049", /* yellow  */
  [12] = "#696057", /* blue    */
  [13] = "#6f5a59", /* magenta */
  [14] = "#525f66", /* cyan    */
  [15] = "#cdcdcd", /* white   */

  /* special colors */
  [256] = "#181b20", /* background */
  [257] = "#9b9081", /* foreground */
};

/*
 * Default colors (colorname index)
 * foreground, background, cursor
 */
unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
