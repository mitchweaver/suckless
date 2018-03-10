const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#3d3e3d", /* black   */
  [1] = "#755a5b", /* red     */
  [2] = "#68755a", /* green   */
  [3] = "#756e5a", /* yellow  */
  [4] = "#5b6976", /* blue    */
  [5] = "#755b76", /* magenta */
  [6] = "#5b7674", /* cyan    */
  [7] = "#808080", /* white   */

  /* 8 bright colors */
  [8]  = "#5a5b5c", /* black   */
  [9]  = "#a37679", /* red     */
  [10] = "#87a376", /* green   */
  [11] = "#a39b76", /* yellow  */
  [12] = "#758ba3", /* blue    */
  [13] = "#9f76a3", /* magenta */
  [14] = "#75a3a1", /* cyan    */
  [15] = "#aeafb0", /* white   */

  /* special colors */
  [256] = "#000000", /* background */
  [257] = "#ffffff", /* foreground */
};

unsigned int defaultfg = 257;
unsigned int defaultbg = 256;
unsigned int defaultcs = 257;
