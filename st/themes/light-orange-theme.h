const char *colorname[] = {

  /* 8 normal colors */
  [0] = "#fcfaf8", /* black   */
  [1] = "#998866", /* red     */
  [2] = "#998d66", /* green   */
  [3] = "#997866", /* yellow  */
  [4] = "#998166", /* blue    */
  [5] = "#668999", /* magenta */
  [6] = "#989966", /* cyan    */
  [7] = "#423F3D", /* white   */

  /* 8 bright colors */
  [8]  = "#7e7d7c",  /* black   */
  [9]  = "#998866",  /* red     */
  [10] = "#998d66", /* green   */
  [11] = "#997866", /* yellow  */
  [12] = "#998166", /* blue    */
  [13] = "#668999", /* magenta */
  [14] = "#989966", /* cyan    */
  [15] = "#423F3D", /* white   */

  /* special colors */
  [256] = "#fcfaf8", /* background */
  [257] = "#423F3D", /* foreground */
  [258] = "#423F3D",     /* cursor */
};

/* Default colors (colorname index)
 * foreground, background, cursor */
 unsigned int defaultbg = 0;
 unsigned int defaultfg = 257;
 unsigned int defaultcs = 258;
