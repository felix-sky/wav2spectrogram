// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef SP_GPLOT_H
#define SP_GPLOT_H
#include <vector>
#include <typeinfo>
namespace sp
{
    ///
    /// @defgroup gplot GPlot
    /// \brief Collection of Gnuplot functions
    /// @{

    ///
    /// \brief Gnuplot class.
    ///
    /// Implements a class for streaming data to Gnuplot using a pipe.
    /// Inspiration from https://code.google.com/p/gnuplot-cpp/
    ///
    /// Requires Gnuplot version > 5.0
    /// \note In Windows only one class is allowed. Using multiple figures are controlled by a figure number. In Linux we may use one instance per figure.
    ///
    class gplot
    {
        private:
            FILE*           gnucmd;          ///< File handle to pipe
            std::string     term;
            int             fig_ix;
            int             plot_ix;

            struct plot_data_s
            {
                std::string label;
                std::string linespec;
            };

            std::vector<plot_data_s> plotlist;

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot y vs. x.
            /// @param x x vector
            /// @param y y vector
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T1, typename T2>
            void plot_str2( const T1& x, const T2& y)
            {
                std::ostringstream tmp_s;
                std::string s;
                tmp_s << "$Dxy" << plot_ix << " << EOD \n";
                arma::uword Nelem = x.n_elem;
                for(arma::uword n=0; n<Nelem; n++)
                {
                    tmp_s << x(n) << " " << y(n);
                    s = tmp_s.str();
                    send2gp(s.c_str());
                    tmp_s.str(""); // Clear buffer
                }
                send2gp("EOD");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Get type
            /// @param x     x input type
            /// @returns     Type name
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            std::string get_type(T x)
            {
                if (typeid(x) == typeid(int8_t))     return "int8";
                if (typeid(x) == typeid(uint8_t))    return "uint8";
                if (typeid(x) == typeid(int16_t))    return "int16";
                if (typeid(x) == typeid(uint16_t))   return "uint16";
                if (typeid(x) == typeid(int32_t))    return "int32";
                if (typeid(x) == typeid(uint32_t))   return "uint32";
                if (typeid(x) == typeid(int64_t))    return "int64";
                if (typeid(x) == typeid(arma::sword))return "int64";
                if (typeid(x) == typeid(uint64_t))   return "uint64";
                if (typeid(x) == typeid(arma::uword))return "uint64";
                if (typeid(x) == typeid(float))      return "float32";
                if (typeid(x) == typeid(double))     return "float64";
                err_handler("Unknown type");
            }

        public:
            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Constructor.
            ///
            /// Opens a pipe to gnuplot program. Make sure it is found/accessable by the system.
            ////////////////////////////////////////////////////////////////////////////////////////////
            gplot()
            {
#if defined(_MSC_VER)
                gnucmd = _popen("gnuplot -persist 2> NUL","wb");
                term = "win";
//#elif defined(_APPLE_)
//            gnucmd = popen("gnuplot -persist &> /dev/null","w");
//#define term "aqua"
#else
                gnucmd = popen("gnuplot -persist","w");
                term = "x11";
#endif
                if(!gnucmd)
                {
                    err_handler("Could not start gnuplot");
                }

                // Set global params
                plot_ix   = 0;
                fig_ix    = 0;
                plotlist.clear();

            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Destructor.
            ////////////////////////////////////////////////////////////////////////////////////////////
            ~gplot()
            {
#if defined(_MSC_VER)
                _pclose(gnucmd);
#else
                pclose(gnucmd);
#endif
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Send command to Gnuplot pipe.
            /// @param cmdstr  Command string
            ////////////////////////////////////////////////////////////////////////////////////////////
            void send2gp(const char* cmdstr)
            {
                std::string s_in(cmdstr);
                std::string tmp=s_in+"\n";
                std::fputs(tmp.c_str(), gnucmd );
//                std::cout << tmp.c_str() << std::endl;
            }

			////////////////////////////////////////////////////////////////////////////////////////////
			/// \brief Flush command buffer to Gnuplot pipe.
			////////////////////////////////////////////////////////////////////////////////////////////
			void flush_cmd_buf(void)
			{
				std::fflush(gnucmd);
			}

			////////////////////////////////////////////////////////////////////////////////////////////
			/// \brief Updates gnuplot instantly. (Flushes the command buffer)
			////////////////////////////////////////////////////////////////////////////////////////////
			void draw_now(void)
			{
				std::fflush(gnucmd);
			}

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Sets the active figure.
            /// @param fig  Figure number
            ////////////////////////////////////////////////////////////////////////////////////////////
            void figure(const int fig)
            {
                fig_ix = fig;
                std::ostringstream tmp_s;
                tmp_s << "set term " << term << " " << fig;
                std::string s = tmp_s.str();
                send2gp(s.c_str());
                send2gp("reset");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Configure the figure used Windows environment.
            /// @param fig     Figure number
            /// @param name    Window name
            /// @param x       x position of upper left corner
            /// @param y       y position of upper left corner
            /// @param width   width of window
            /// @param height  height of window
            ////////////////////////////////////////////////////////////////////////////////////////////
            void window(const int fig, const char* name,const int x,const int y,const int width,const int height)
            {
                fig_ix = fig;
                std::ostringstream tmp_s;
                tmp_s << "set term " << term << " " << fig << " title \"" << name << "\" position " << x << "," << y << " size " << width << "," << height;
                std::string s = tmp_s.str();
                send2gp(s.c_str());
                send2gp("reset");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Configure the figure/window - used in Linux environment where no figure numbers are needed.
            /// @param name    Window name
            /// @param x       x position of upper left corner
            /// @param y       y position of upper left corner
            /// @param width   width of window
            /// @param height  height of window
            ////////////////////////////////////////////////////////////////////////////////////////////
            void window(const char* name,const int x,const int y,const int width,const int height)
            {
                window(0,name,x,y,width,height);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Close window
            ////////////////////////////////////////////////////////////////////////////////////////////
            void close_window(void)
            {
                std::ostringstream tmp_s;
                tmp_s << "set term " << term << " close";
                std::string s = tmp_s.str();
                send2gp(s.c_str());
                send2gp("reset");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set grid
            ////////////////////////////////////////////////////////////////////////////////////////////
            void grid_on(void)
            {
                send2gp("set grid");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set grid
            ////////////////////////////////////////////////////////////////////////////////////////////
            void grid_off(void)
            {
                send2gp("unset grid");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set label for X-axis.
            /// @param label label string
            ////////////////////////////////////////////////////////////////////////////////////////////
            void xlabel(const char* label)
            {
                std::ostringstream tmp_s;
                tmp_s << "set xlabel \"" << label << "\" ";
                std::string s = tmp_s.str();
                send2gp(s.c_str());
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set label for X-axis.
            /// @param label label string
            ////////////////////////////////////////////////////////////////////////////////////////////
            void ylabel(const char* label)
            {
                std::ostringstream tmp_s;
                tmp_s << "set ylabel \"" << label << "\" ";
                std::string s = tmp_s.str();
                send2gp(s.c_str());
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set label at position x,y.
            /// @param x x value
            /// @param y y value
            /// @param label label string
            ////////////////////////////////////////////////////////////////////////////////////////////
            void label(const double& x, const double& y, const char* label)
            {
                std::ostringstream tmp_s;
                tmp_s << "set label \"" << label << "\" at " << x << "," << y;
                std::string s = tmp_s.str();
                send2gp(s.c_str());
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set windowtitle.
            /// @param name title string
            ////////////////////////////////////////////////////////////////////////////////////////////
            void title(const char* name)
            {
                std::ostringstream tmp_s;
                tmp_s << "set title \"" << name << " \" ";
                std::string s = tmp_s.str();
                send2gp(s.c_str());
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set X-axis range.
            /// @param xmin xmin
            /// @param xmax xmax
            ////////////////////////////////////////////////////////////////////////////////////////////
            void xlim(const double xmin, const double xmax)
            {
                std::ostringstream tmp_s;
                tmp_s << "set xrange [" << xmin << ":" << xmax << "]";
                std::string s = tmp_s.str();
                send2gp(s.c_str());
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set Y-axis range.
            /// @param ymin ymin
            /// @param ymax ymax
            ////////////////////////////////////////////////////////////////////////////////////////////
            void ylim(const double ymin, const double ymax)
            {
                std::ostringstream tmp_s;
                tmp_s << "set yrange [" << ymin << ":" << ymax << "]";
                std::string s = tmp_s.str();
                send2gp(s.c_str());
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Push plot y vs. x with label and linespec
            /// @param x      x vector
            /// @param y      y vector
            /// @param lb     label
            /// @param ls     line spec
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T1, typename T2>
            void plot_add( const T1& x, const T2& y, const std::string lb, const std::string ls="lines")
            {
                plot_data_s pd;

                pd.linespec = ls;
                pd.label    = lb;

                plotlist.push_back(pd);
                plot_str2(x,y);
                plot_ix++;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Push plot y vs. x with label and linespec
            /// @param y      y vector
            /// @param lb     label
            /// @param ls     line spec
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T1>
            void plot_add( const T1& y, const std::string lb, const std::string ls="lines")
            {
                arma::vec x=arma::regspace(0,double(y.n_elem-1));
                plot_data_s pd;

                pd.linespec = ls;
                pd.label    = lb;

                plotlist.push_back(pd);
                plot_str2(x,y);
                plot_ix++;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Push multiple plot, each row gives a plot without label
            /// @param y      y matrix
            ////////////////////////////////////////////////////////////////////////////////////////////
            void plot_add_mat( const arma::mat& y)
            {
                arma::vec x=arma::regspace(0,double(y.n_cols-1));
                plot_data_s pd;

                pd.linespec = "lines";
                pd.label    = "";
                for(arma::uword r=0;r<y.n_rows;r++)
                {
                    plotlist.push_back(pd);
                    plot_str2(x,y.row(r));
                    plot_ix++;
                }
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Push multiple plot, each row gives a plot with prefix label
            /// @param y      y matrix
            /// @param p_lb   Label prefix
            ////////////////////////////////////////////////////////////////////////////////////////////
            void plot_add_mat( const arma::mat& y, const std::string p_lb)
            {
                arma::vec x=arma::regspace(0,double(y.n_cols-1));
                plot_data_s pd;
                pd.linespec = "lines";

                for(arma::uword r=0;r<y.n_rows;r++)
                {
                    std::ostringstream tmp_s;
                    tmp_s << p_lb << r;
                    std::string s = tmp_s.str();
                    pd.label = s;
                    plotlist.push_back(pd);
                    plot_str2(x,y.row(r));
                    plot_ix++;
                }
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Show plots
            ////////////////////////////////////////////////////////////////////////////////////////////
            void plot_show(void)
            {
                std::ostringstream tmp_s;

                tmp_s << "plot $Dxy0 title \"" << plotlist[0].label << "\" with " << plotlist[0].linespec;
                for(int r=1; r<plot_ix; r++)
                {
                    tmp_s << " ,$Dxy" << r <<" title \"" << plotlist[r].label << "\" with " << plotlist[r].linespec;
                }
                std::string s = tmp_s.str();
                send2gp(s.c_str());
                plotlist.clear();
                plot_ix = 0;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Clear plots
            ////////////////////////////////////////////////////////////////////////////////////////////
            void plot_clear(void)
            {
                plotlist.clear();
                plot_ix = 0;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot vector, fast version
            /// @param x     x vector
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            void fast_plot( const arma::Col<T>& x, const std::string fmt_args = "with lines")
            {
                std::string fmt=get_type(x.at(0));
                std::string s;
                send2gp("unset key");

                s= "plot '-' binary format='%"+
                   fmt + "' array=("+std::to_string(x.n_elem)+") "+fmt_args;
                send2gp(s.c_str());
                std::fwrite(x.memptr(), sizeof(x.at(0)),x.n_elem,gnucmd );
                flush_cmd_buf();
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot vector, fast version
			///
			/// x and y needs to have the same type
            /// @param x     x vector
			/// @param y     y vector
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            void fast_plot( const arma::Col<T>& x, const arma::Col<T>& y, const std::string fmt_args = "with lines")
            {
                std::string fmt1=get_type(x.at(0));
                std::string s;
                send2gp("unset key");
                const arma::uword N=x.n_elem;
                arma::Col<T> v(2*N);
                for(arma::uword n=0;n<N;n++)
                {
                    v.at(2*n) = x.at(n);
                    v.at(2*n+1) = y.at(n);
                }

                s= "plot '-' binary format='%"+fmt1+"' record=("+std::to_string(x.n_elem)+ ") "+fmt_args;
                send2gp(s.c_str());
                std::fwrite(v.memptr(), sizeof(x.at(0)),v.n_elem,gnucmd );
                flush_cmd_buf();
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot mat as image
            /// @param x     x matrix
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            void image( const arma::Mat<T>& x)
            {
                xlim(-0.5,x.n_cols-0.5);
                ylim(x.n_rows-0.5,-0.5);
                send2gp("unset key");
                std::string fmt=get_type(x.at(0));
                std::string s;

                s= "plot '-' binary array=(" +
                   std::to_string(x.n_cols) + "," +
                   std::to_string(x.n_rows) + ") scan=yx format='%" +
                   fmt + "' w image";
                send2gp(s.c_str());
                std::fwrite(x.memptr(), sizeof(x.at(0)),x.n_elem,gnucmd );
                flush_cmd_buf();
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot cube as image
            /// @param x     x matrix (R,G,B)
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            void image( const arma::Cube<T>& x)
            {
                xlim(-0.5,x.n_cols-0.5);
                ylim(x.n_rows-0.5,-0.5);
                send2gp("unset key");
                std::string fmt=get_type(x.at(0));
                std::string s;

                // Conv cube to gnuplot rgb array
                arma::Cube<T> gp_im(arma::size(x));
                T* ptr=gp_im.memptr();
                for(arma::uword c=0;c<x.n_cols; c++ )
                {
                    for(arma::uword r=0;r<x.n_rows;  r++ )
                    {
                        *ptr++ = x.at(r,c,0);    // R
                        *ptr++ = x.at(r,c,1);    // G
                        *ptr++ = x.at(r,c,2);    // B
                    }
                }

                s= "plot '-' binary array=(" +
                   std::to_string(x.n_cols) + "," +
                   std::to_string(x.n_rows) + ") scan=yx format='%" +
                   fmt+ "' w rgbimage";
                send2gp(s.c_str());
                std::fwrite(gp_im.memptr(), sizeof(x.at(0)),gp_im.n_elem,gnucmd );
                flush_cmd_buf();
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot mat as mesh
            /// @param x     x matrix
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            void mesh( const arma::Mat<T>& x)
            {
                send2gp("unset key");
                send2gp("set hidden3d");
                std::string fmt=get_type(x.at(0));
                std::string s= "splot '-' binary array=(" +
                   std::to_string(x.n_cols) + "," +
                   std::to_string(x.n_rows) + ") scan=yx format='%" +
                   fmt+ "' w lines";
                send2gp(s.c_str());
                std::fwrite(x.memptr(),sizeof(x.at(0)),x.n_elem,gnucmd );
                flush_cmd_buf();
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Plot mat as surf
            /// @param x     x matrix
            ////////////////////////////////////////////////////////////////////////////////////////////
            template <typename T>
            void surf( const arma::Mat<T>& x)
            {
                send2gp("set pm3d");
                mesh(x);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set linetype to Matlab 'parula' NB! doesn't work with X11 -terminal
            /// Data from https://github.com/Gnuplotting/gnuplot-palettes
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_parula_line(void)
            {
                send2gp("set linetype 1 lc rgb '#0072bd' "); // blue
                send2gp("set linetype 2 lc rgb '#d95319' "); // orange
                send2gp("set linetype 3 lc rgb '#edb120' "); // yellow
                send2gp("set linetype 4 lc rgb '#7e2f8e' "); // purple
                send2gp("set linetype 5 lc rgb '#77ac30' "); // green
                send2gp("set linetype 6 lc rgb '#4dbeee' "); // light-blue
                send2gp("set linetype 7 lc rgb '#a2142f' "); // red
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set linetype to Matlab 'jet' NB! doesn't work with X11 -terminal
            /// Data from https://github.com/Gnuplotting/gnuplot-palettes
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_jet_line(void)
            {
                send2gp("set linetype 1 lc rgb '#0000ff' "); // blue
                send2gp("set linetype 2 lc rgb '#007f00' "); // green
                send2gp("set linetype 3 lc rgb '#ff0000' "); // red
                send2gp("set linetype 4 lc rgb '#00bfbf' "); // cyan
                send2gp("set linetype 5 lc rgb '#bf00bf' "); // pink
                send2gp("set linetype 6 lc rgb '#bfbf00' "); // yellow
                send2gp("set linetype 7 lc rgb '#3f3f3f' "); // black
            }
            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set linetype to Matlab 'parula' NB! doesn't work with X11 -terminal
            /// Data from https://github.com/Gnuplotting/gnuplot-palettes
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_set1_line(void)
            {
                send2gp("set linetype 1 lc rgb '#E41A1C' ");// red
                send2gp("set linetype 2 lc rgb '#377EB8' ");// blue
                send2gp("set linetype 3 lc rgb '#4DAF4A' ");// green
                send2gp("set linetype 4 lc rgb '#984EA3' ");// purple
                send2gp("set linetype 5 lc rgb '#FF7F00' ");// orange
                send2gp("set linetype 6 lc rgb '#FFFF33' ");// yellow
                send2gp("set linetype 7 lc rgb '#A65628' ");// brown
                send2gp("set linetype 8 lc rgb '#F781BF' ");// pink

                send2gp("set palette maxcolors 8");
                char str[] ="set palette defined ( \
                      0 '#E41A1C',\
                      1 '#377EB8',\
                      2 '#4DAF4A',\
                      3 '#984EA3',\
                      4 '#FF7F00',\
                      5 '#FFFF33',\
                      6 '#A65628',\
                      7 '#F781BF')";
                send2gp(str);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set palette to Matlab 'jet'
            /// Data from https://github.com/Gnuplotting/gnuplot-palettes
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_jet_palette(void)
            {
                char str[] ="set palette defined ( \
                      0 '#000090',\
                      1 '#000fff',\
                      2 '#0090ff',\
                      3 '#0fffee',\
                      4 '#90ff70',\
                      5 '#ffee00',\
                      6 '#ff7000',\
                      7 '#ee0000',\
                      8 '#7f0000')";
                send2gp(str);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set palette to Matlab 'parula'
            /// Data from https://github.com/Gnuplotting/gnuplot-palettes
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_parula_palette(void)
            {
                char str[] ="set palette defined (\
                      0 '#352a87',\
                      1 '#0363e1',\
                      2 '#1485d4',\
                      3 '#06a7c6',\
                      4 '#38b99e',\
                      5 '#92bf73',\
                      6 '#d9ba56',\
                      7 '#fcce2e',\
                      8 '#f9fb0e')";
                send2gp(str);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set palette to Matlab 'parula'
            /// Data from https://github.com/Gnuplotting/gnuplot-palettes
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_custom_palette(void)
            {
                char str[] ="set palette defined (\
                      0 '#352a87',\
                      1 '#0363e1',\
                      2 '#1485d4',\
                      3 '#06a7c6',\
                      4 '#38b99e',\
                      5 '#92bf73',\
                      6 '#d9ba56',\
                      7 '#fec635',\
                      8 '#f9f916')";
                send2gp(str);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set palette to 'cool-warm'
            // See http://www.kennethmoreland.com/color-advice/
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_coolwarm_palette(void)
            {
                char str[] = "set palette defined (\
                      0 '#5548C1', \
                      1 '#7D87EF', \
                      2 '#A6B9FF', \
                      3 '#CDD7F0', \
                      4 '#EBD1C2', \
                      5 '#F3A889', \
                      6 '#DE6A53', \
                      7 '#B10127')";
                send2gp(str);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set palette to 'black body'
            // See http://www.kennethmoreland.com/color-advice/
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_blackbody_palette(void)
            {
                char str[] = "set palette defined (\
                      0 '#000000', \
                      1 '#2B0F6B', \
                      2 '#5D00CB', \
                      3 '#C60074', \
                      4 '#EB533C', \
                      5 '#F59730', \
                      6 '#E9D839', \
                      7 '#FFFFFF')";
                send2gp(str);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Save plot to file.
            /// @param name filename
            ///
            /// Extensions that are supported:
            /// - png
            /// - ps
            /// - eps
            /// - tex
            /// - pdf
            /// - svg
            /// - emf
            /// - gif
            ///
            /// \note When 'latex' output is used the '\' must be escaped by '\\\\' e.g set_xlabel("Frequency $\\\\omega = 2 \\\\pi f$")
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_output(const char* name)
            {
                std::string name_s(name);
                size_t found = name_s.find_last_of(".");
                std::string ext;
                ext = name_s.substr(found + 1);
                std::ostringstream tmp_s;

                if (ext.compare("png")==0)
                {
                    tmp_s << "set terminal pngcairo enhanced font 'Verdana,10'";
                }
                else if (ext.compare("ps") == 0)
                {
                    tmp_s << "set terminal postscript enhanced color";
                }
                else if (ext.compare("eps") == 0)
                {
                    tmp_s << "set terminal postscript eps enhanced color";
                }
                else if (ext.compare("tex") == 0)
                {
                    tmp_s << "set terminal cairolatex eps color enhanced";
                }
                else if (ext.compare("pdf") == 0)
                {
                    tmp_s << "set terminal pdfcairo color enhanced";
                }
                else if (ext.compare("svg") == 0)
                {
                    tmp_s << "set terminal svg enhanced";
                }
                else if (ext.compare("emf") == 0)
                {
                    tmp_s << "set terminal emf color enhanced";
                }
                else if (ext.compare("gif") == 0)
                {
                    tmp_s << "set terminal gif enhanced";
                }
                //else if (ext.compare("jpg") == 0)
                //{
                //	tmp_s << "set terminal jpeg ";
                //}
                else
                {
                    tmp_s << "set terminal " << term;
                }
                std::string s = tmp_s.str();
                send2gp(s.c_str());
                tmp_s.str("");  // Clear buffer
                tmp_s << "set output '" << name_s << "'";
                s = tmp_s.str();
                send2gp(s.c_str());
            }


            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Reset output terminal.
            ////////////////////////////////////////////////////////////////////////////////////////////
            void reset_term(void)
            {
                send2gp("reset session");
            }

            ////////////////////////////////////////////////////////////////////////////////////////////
            /// \brief Set output terminal.
            ////////////////////////////////////////////////////////////////////////////////////////////
            void set_term(const char* ttype)
            {
                std::ostringstream tmp_s;
                tmp_s << "set terminal " << ttype;
                std::string s = tmp_s.str();
                term = s;
                send2gp(s.c_str());
            }

    }; // End Gnuplot Class


    /// @}

} // end namespace
#endif
