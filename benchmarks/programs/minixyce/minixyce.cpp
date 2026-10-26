/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : Sept 2018

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#ifndef __MX_COMM_DEFS_H__
#define __MX_COMM_DEFS_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <list>

namespace mX_comm_utils
{
  struct data_transfer_instruction
  {
    std::list<int> indices;    // which elements of the vector to send
    int pid;    // to which processor the data is to be sent

    data_transfer_instruction(int pid_in)
      : pid( pid_in )
    {}
  };
}

#endif

#ifndef __MX_PARSER_H__
#define __MX_PARSER_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <string>
#include <map>
#ifndef __MX_SOURCE_DEFS_H__
#define __MX_SOURCE_DEFS_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <vector>
#include <sstream>

namespace mX_source_utils
{
	class mX_source
	{
		// this class represents a time-varying voltage or current source
			// all that is needed is the value of the output at time "t"

		public:

                virtual ~mX_source() {}
		virtual double output(double t) = 0;
	};

	class DC : public mX_source
	{
		// DC voltage/current source

		public:

		double val;

		DC(double d)
		{
			val = d;
		}

		virtual double output(double t)
		{
			return val;
		}
	};

	class SINE : public mX_source
	{
		// sinusoidal voltage/current source

		public:

		double offset;
		double amplitude;
		double freq;
		double phase;

		SINE(double off, double amp, double f, double ph);

		virtual double output(double t); 
	};

	class FM : public mX_source
	{
		// frequency modulated voltage/current source

		public:

		double offset;
		double amplitude;
		double carrier_freq;
		double modulation_index;
		double signal_freq;

		FM(double o, double a, double cf, double mi, double sf);

		virtual double output(double t); 
	};

	class PWL : public mX_source
	{
		// piecewise linear voltage/current source

		public:

		std::vector<double> times;
		std::vector<double> values;

		PWL(std::vector<double> ts, std::vector<double> vals);

		virtual double output(double t);
	};

	class PULSE : public mX_source
	{
		// pulse shaped voltage/current source

		public:

		double val1;
		double val2;
		double t_delay;
		double t_rise;
		double pulse_width;
		double t_fall;
		double t_stop;

		PULSE(double d1, double d2, double td, double tr, double pw, double tf, double ts);

		virtual double output(double t);
	};

	struct mX_scaled_source
	{
		// a scaled voltage/current source
			// useful in many contexts
				// eg: when you want -V(t) and already have a source V(t)

		mX_source* src;
		double scale;
	};

	mX_source* parse_source(std::istringstream& input_str);
}
#endif

#ifndef __MX_LINEAR_DAE_H__
#define __MX_LINEAR_DAE_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <map>
#include <list>
#include <vector>

#ifndef __MX_SPARSE_MATRIX_DEFS_H__
#define __MX_SPARSE_MATRIX_DEFS_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <vector>
#include <list>
#include <map>

namespace mX_DAE_utils
{
  struct mX_DAE_RHS_entry;
}

namespace mX_comm_utils
{
  struct data_transfer_instruction;
}

namespace mX_vector_utils
{
  struct distributed_vector;
}
 
namespace mX_matrix_utils
{
  struct distributed_sparse_matrix_entry
  {
    int column;    // global column index
    double value;  // value stored in the matrix
    distributed_sparse_matrix_entry* next_in_row;  // pointer to next entry in the same row

    distributed_sparse_matrix_entry()
      : column(-1), value(0.0), next_in_row(0)
    {}

    distributed_sparse_matrix_entry(int col_in, double val_in, distributed_sparse_matrix_entry* nir_in)
      : column( col_in ), value( val_in ), next_in_row( nir_in )
    {}
  };

  struct distributed_sparse_matrix
  {
    // the data structure for a distributed sparse matrix is a 1-d threaded list
      // a set of pointers called row_headers point to the first entry of each row
      // each row entry in turn points to the next entry in the same row

    // but a distributed matrix needs more data than this
      // there is a list of data transfer instructions
        // these instructions are to be followed whenever a mat-vec product is needed
    
    // each processor also stores 2 entries start_row and end_row
      // it is assumed that all processors store contiguous rows of the distributed matrix

    int start_row;
    int end_row;
    int local_nnz;
    std::vector<int> local_rows;
    std::vector<int> overlap_rows, overlap_recv, overlap_send;

    int n;
    int p;      // total number of parallel processors
    int my_pid; // this processors id

    distributed_sparse_matrix_entry ground_node;
    std::vector<distributed_sparse_matrix_entry*> row_headers;
    std::list<mX_comm_utils::data_transfer_instruction*> send_instructions;
    std::list<mX_comm_utils::data_transfer_instruction*> recv_instructions;

    distributed_sparse_matrix();

    distributed_sparse_matrix(int total_unknowns, int num_procs, int pid);
  };

  distributed_sparse_matrix_entry* distributed_sparse_matrix_insert(distributed_sparse_matrix* M,
                                                                    int row_idx, int col_idx);

  void distributed_sparse_matrix_insert(distributed_sparse_matrix* M, int row_idx, int col_idx, double val);

  void distributed_sparse_matrix_finish(distributed_sparse_matrix* A, distributed_sparse_matrix* B,
                                        const std::vector<mX_DAE_utils::mX_DAE_RHS_entry*>& b);

  void distributed_sparse_matrix_add_to(distributed_sparse_matrix* M, int row_idx, int col_idx, double val);

  void sparse_matrix_vector_product(distributed_sparse_matrix* A, mX_vector_utils::distributed_vector& x, mX_vector_utils::distributed_vector& y);

  void sparse_matrix_vector_product(distributed_sparse_matrix* A, std::vector<double> &x, std::vector<double> &y);

  void destroy_matrix(distributed_sparse_matrix* A);

  void print_matrix(distributed_sparse_matrix &A);

  void init_value(distributed_sparse_matrix& A, double val);
}


#endif

#ifndef __MX_VECTOR_DEFS_H__
#define __MX_VECTOR_DEFS_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <vector>
#include <list>
#include <map>

namespace mX_comm_utils
{
  struct data_transfer_instruction;
}

namespace mX_vector_utils
{
  struct distributed_vector
  {
    // this is a distributed vector using the same data transfer information from
      // the matrix and the row distribution from the matrix.

    std::vector<int> local_rows, overlap_rows;
    std::map<int,int> row_to_idx;
    std::vector<double> values;

    int n;
    int p;      // total number of parallel processors
    int my_pid; // this processors id

    std::list<mX_comm_utils::data_transfer_instruction*> send_instructions;
    std::list<mX_comm_utils::data_transfer_instruction*> recv_instructions;

    distributed_vector(int n_in, int p_in, int my_pid_in,
                       const std::vector<int>& local_rows_in, const std::vector<int>& overlap_rows_in,
                       const std::list<mX_comm_utils::data_transfer_instruction*>& send_inst_in,
                       const std::list<mX_comm_utils::data_transfer_instruction*>& recv_inst_in)
    : n( n_in ),
      p( p_in ),
      my_pid( my_pid_in ),
      local_rows( local_rows_in ),
      overlap_rows( overlap_rows_in ),
      send_instructions( send_inst_in ),
      recv_instructions( recv_inst_in )
    {
      int idx = 0;
      for (std::vector<int>::iterator it = local_rows.begin(); it != local_rows.end(); it++, idx++)
        row_to_idx[ *it ] = idx;
      for (std::vector<int>::iterator it = overlap_rows.begin(); it != overlap_rows.end(); it++, idx++)
        row_to_idx[ *it ] = idx;
      
      values.resize( row_to_idx.size() );
    }
  };

  void axpy(double alpha, const distributed_vector& x, double beta, distributed_vector& y);

  void scale(distributed_vector& x, double alpha);
 
  double norm2(distributed_vector& v);

  double dot(distributed_vector& x, distributed_vector& y);

  void print_vector(distributed_vector& v);
 
  void assemble_vector(distributed_vector& x);

  void init_value(distributed_vector& x, double val);

}

#endif

#ifndef __MX_DEVICE_DEFS_H__
#define __MX_DEVICE_DEFS_H__

//@HEADER
// ************************************************************************
//
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2018


#include <sstream>
#include <string>
#include <vector>

using namespace mX_vector_utils;
using namespace mX_matrix_utils;

namespace mX_DAE_utils
{
  struct mX_DAE;
}


namespace mX_device_utils
{

class mX_device
{
        // this class represents a device model, that can be time-invariant
                // or time-varying.
        public:

        mX_device( char label ) { label_ = label; }
        virtual ~mX_device() {}

        // Add a device described by this input stream, returns number of internal nodes required 
        virtual int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae ) { return 0; }

        // Return number of internal nodes introduced by this device type.
        virtual std::vector<std::pair<int,int> >& get_jacobian_stamp() { return jac_stamp_; }

        // Evaluate devices
        virtual void load_vector( distributed_vector& vec ) {}
        virtual void load_matrices() {}

        // Some indication of flops for evaluating this device model.
        virtual int modelCost() { return 0; }

        // Character (letter) indicating which device type.
        virtual char deviceLabel() { return label_; }

        protected:

        std::vector<distributed_sparse_matrix_entry* > device_entries_;

        private:

        char label_;
        std::vector<std::pair<int,int> > jac_stamp_;
};

mX_device* create_device( char device_label );

void add_device_nodes( char device_label, std::istringstream& input_str, std::vector<int>& node_list );

int num_internal_nodes( char device_label, std::istringstream& input_str );
}

#endif // __MX_DEVICE_DEFS_H__


using namespace mX_source_utils;
using namespace mX_matrix_utils;
using namespace mX_vector_utils;
using namespace mX_device_utils;

namespace mX_DAE_utils
{
  struct mX_DAE_RHS_entry
  {
    // a single entry in the RHS of a DAE of the form "A x + B x_dot = b(t)"
      // each entry in b(t) is a list of scaled sources
        // linear combination of multiple voltage/current sources

    std::list<mX_scaled_source*> scaled_src_list;
  };

  struct mX_DAE
  {
    // a linear DAE is of the form "A x + B x_dot = b(t)"
      // A and B are distributed sparse matrices
      // b(t) is a time-varying function
        // every entry of which is a linear combination of voltage/current sources

    distributed_sparse_matrix* A;
    distributed_sparse_matrix* B;
    std::vector<mX_DAE_RHS_entry*> b;

    std::map<char, mX_device*> devices;
  };

  distributed_vector evaluate_b(double t, mX_DAE* dae);
  std::vector<double> evaluate_b_old(double t, mX_DAE* dae);
  void load_matrices(mX_DAE* dae);
  void destroy(mX_DAE* dae);
  void destroy_RHS(mX_DAE_RHS_entry* entry);
}

#endif



using namespace mX_source_utils;
using namespace mX_matrix_utils;
using namespace mX_DAE_utils;

namespace mX_parse_utils
{
	mX_DAE* parse_netlist(std::string filename, int p, int pid, int &total_devices, int &total_unknowns, int &num_external_nodes, std::map<char, int>& device_count);
}
#endif



#ifndef __MX_SOLVER_DEFS_H__
#define __MX_SOLVER_DEFS_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : Sept 2018

#include <vector>
#include <list>
#include <map>

namespace mX_comm_utils
{
  struct data_transfer_instruction;
}

namespace mX_vector_utils
{
  struct distributed_vector;
}
 
namespace mX_matrix_utils
{
  struct distributed_sparse_matrix;
}

namespace mX_solver_utils
{
  void gmres(mX_matrix_utils::distributed_sparse_matrix* A, mX_vector_utils::distributed_vector &b, mX_vector_utils::distributed_vector &x,
             double &tol, double &err, int k, int &iters, int &restarts);

  void rotg(double& a, double& b, double& c, double& s);
}

#endif


#ifndef __MX_PARMS_H__
#define __MX_PARMS_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <string>
#include <set>
#include <vector>

namespace mX_parms_utils
{
  enum parm_names{CKT_FILENAME,T_START,T_STEP,T_STOP,TOL,K,MAX_RESTART,INIT_COND,PARMS_FILE,PREV};

  void parse_command_line(int argc, std::vector<std::string> &argv, std::string &ckt_filename, double &t_start, double &t_step, double &t_stop, double &tol, int &k, int& max_restarts, 
                          std::vector<double> &init_cond, std::string &parms_file, std::set<int> &specified_parms, int p, int pid);

  std::vector<std::string> get_command_line_equivalent_from_file(std::string &filename);

  void get_parms(int argc, char* argv[], std::string &ckt_filename, double &t_start, double &t_step, double &t_stop, double &tol, int &k, int &max_restarts,
                 std::vector<double> &init_cond, bool &init_cond_specified, int p, int pid);
}

#endif

#ifndef __MX_TIMER_H__
#define __MX_TIMER_H__

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

double mX_timer(void);
#endif // __MX_TIMER_H__

//@HEADER
// ************************************************************************
// 
//               Mantevo: A collection of mini-applications for HPC
//                 Copyright (2008) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Changelog
//
// Version 0.1
// - Initial version.
//
/////////////////////////////////////////////////////////////////////////

#ifndef YAML_ELEMENT_H
#define YAML_ELEMENT_H
#include <string>
#include <vector>
//! The Mantevo YAML_Element class for registering key-value pairs of performance data

/*!
  Mantevo mini-applications generate a collection of performance data for each run of the executable.  YAML_Element, and
  the related YAML_Doc class, provide a uniform facility for gathering and reporting this data using the YAML text format.
*/
class YAML_Element {
  public:

  //! Default constructor.
  YAML_Element (){key="";value="";}
  //! Construct with known key-value pair
  YAML_Element (const std::string& key_arg, const std::string& value_arg);
  //! Destructor
  ~YAML_Element ();
  //! Key accessor method
  std::string getKey(){return key;}
  //! Add a child element to an element list associated with this element, value of type double
  YAML_Element* add(const std::string& key_arg, double value_arg);
  //! Add a child element to an element list associated with this element, value of type int
  YAML_Element* add(const std::string& key_arg, int value_arg);
#ifndef MINIFE_NO_LONG_LONG
  //! Add a child element to an element list associated with this element, value of type long long
  YAML_Element* add(const std::string& key_arg, long long value_arg);
#endif
  //! Add a child element to an element list associated with this element, value of type size_t
  YAML_Element* add(const std::string& key_arg, size_t value_arg);
  //! Add a child element to an element list associated with this element, value of type string
  YAML_Element* add(const std::string& key_arg, const std::string& value_arg);
  //! get the element in the list with the given key
  YAML_Element* get(const std::string& key_arg);
  std::string printYAML(std::string space);
  
protected:
  std::string key;
  std::string value;
  std::vector<YAML_Element*> children;

private:
  std::string convert_double_to_string(double value_arg);
  std::string convert_int_to_string(int value_arg);
#ifndef MINIFE_NO_LONG_LONG
  std::string convert_long_long_to_string(long long value_arg);
#endif
  std::string convert_size_t_to_string(size_t value_arg);
};
#endif /* YAML_ELEMENT_H */

//@HEADER
// ************************************************************************
// 
//               Mantevo: A collection of mini-applications for HPC
//                 Copyright (2008) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Changelog
//
// Version 0.1
// - Initial version.
//
/////////////////////////////////////////////////////////////////////////

#ifndef YAML_DOC_H
#define YAML_DOC_H
#include <string>
#include <vector>


//! The Mantevo YAML_Doc class for the uniform collecting and reporting of performance data for mini-applications

/*!

The YAML_Doc class works in conjuction with the YAML_Element class to facilitate easy collecting and reporting of YAML-formatted
data that can be then registered with the Mantevo results collection website.

\code

//EXAMPLE CODE FOR GENERATING YAML

  YAML_Doc doc("hpccg","1.0");
  doc.add("final_residual",1.4523e-13);
  doc.add("time","4.893"); 
 
//note: the following line will remove the data (4.890) associated with "time"
  doc.get("time")->add("total",4.243);

//note:  the following line will likewise remove the data (1.243) associated with "time"
  doc.get("time")->get("total")->add("time",2.457);
  doc.get("time")->get("total")->add("flops",4.88e5);
  doc.get("time")->add("ddot",1.243);
  doc.get("time")->add("sparsemv","");
  doc.get("time")->get("sparsemv")->add("time",0.3445);
  doc.get("time")->get("sparsemv")->add("overhead","");
  doc.get("time")->get("sparsemv")->get("overhead")->add("time",0.0123);
  doc.get("time")->get("sparsemv")->get("overhead")->add("percentage",0.034);
  cout << doc.generateYAML() << endl; 
  return 0;

\endcode

Below is the output generated by the above code:

\verbatim

final_residual: 1.4523e-13
time: 
  total:
    time: 2.457
    flops: 4.88e5
  ddot: 1.243
  sparsemv:
    time: 0.3445
    overhead:
      time: 0.0123
      percentage: 0.034

\endverbatim

\note {No value is allowed to be attached to a key that has children.  If children are added to a key, the value is simply set to "".}

*/
class YAML_Doc: public YAML_Element {
  public:
  //! Constructor: accepts mini-application name and version as strings, optionally accepts directory and file name for printing results.
  /*!
    The sole constructor for this class accepts and name and version number for the mini-application as well as optional directory 
    and file name information for results that are generated by the generateYAML() method.
    \param miniApp_Name (in) string containing name of the mini-application
    \param miniApp_Version (in) string containing the version of the mini-application
    \param destination_Directory (in, optional) path of diretory where results file will be stored, relative to current working directory. 
           If this value is not supplied, the results file will be stored in the current working directory.  If the directory does not exist
	   it will be created.
    \param destination_FileName (in, optional) root name of the results file.  A suffix of ".yaml" will be automatically appended.  If no
           file name is specified the filename will be constructed by concatenating the miniAppName + miniAppVersion + ".yaml" strings.
  */
  YAML_Doc(const std::string& miniApp_Name, const std::string& miniApp_Version, const std::string& destination_Directory = "", const std::string& destination_FileName = "");
  //! Destructor
  ~YAML_Doc();
  //! Generate YAML results to standard out and to a file using specified directory and filename, using current directory and miniAppName + miniAppVersion + ".yaml" by default
  std::string generateYAML();

protected:
  std::string miniAppName;
  std::string miniAppVersion;
  std::string destinationDirectory;
  std::string destinationFileName;
};
#endif /* YAML_DOC_H */




#ifdef HAVE_MPI
#include "mpi.h"
#endif

using namespace mX_comm_utils;
using namespace mX_parse_utils;
using namespace mX_source_utils;
using namespace mX_DAE_utils;
using namespace mX_parms_utils;
using namespace mX_solver_utils;

int main(int argc, char* argv[])
{
  // this is of course, the actual transient simulator
  int p=1, pid=0;
#ifdef HAVE_MPI  
  MPI_Init(&argc,&argv);
  
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
#endif
  double sim_start = mX_timer();

  // initialize YAML doc
  YAML_Doc doc("miniXyce","1.1");

  // initialize the simulation parameters

  std::string ckt_netlist_filename;
  double t_start, t_step, t_stop, tol, res;
  int k=0, iters=0, restarts=0;

  std::vector<double> x;
  bool init_cond_specified;

  double tstart = mX_timer();
  get_parms(argc,argv,ckt_netlist_filename,t_start,t_step,t_stop,tol,k,restarts,x,init_cond_specified,p,pid);
  double tend = mX_timer() - tstart;
  doc.add("Parameter_parsing_time",tend);

  // build the DAE from the circuit netlist

  tstart = mX_timer();
  int total_devices, total_unknowns, num_external_nodes;
  std::map<char, int> device_count;

  mX_DAE* dae = parse_netlist(ckt_netlist_filename,p,pid,total_devices,total_unknowns,num_external_nodes,device_count);

  tend = mX_timer() - tstart;
  doc.add("Netlist_parsing_time",tend);

  // document circuit and matrix attributes
 
  doc.add("Netlist_file",ckt_netlist_filename.substr(ckt_netlist_filename.find_last_of('/')+1).c_str());

  doc.add("Circuit_attributes","");
  doc.get("Circuit_attributes")->add("Number_of_devices",total_devices);
  if (device_count.find('R') != device_count.end())
  {
    int num_resistors = device_count['R'];
    doc.get("Circuit_attributes")->add("Resistors_(R)",num_resistors);
  }
  if (device_count.find('L') != device_count.end())
  {
    int num_inductors = device_count['L'];
    doc.get("Circuit_attributes")->add("Inductors_(L)",num_inductors);
  }
  if (device_count.find('C') != device_count.end())
  {
    int num_capacitors = device_count['C'];
    doc.get("Circuit_attributes")->add("Capacitors_(C)",num_capacitors);
  }
  if (device_count.find('D') != device_count.end())
  {
    int num_diodes = device_count['D'];
    doc.get("Circuit_attributes")->add("Diodes_(D)",num_diodes);
  if (device_count.find('V') != device_count.end())
  {
    int num_voltage_sources = device_count['V'];
    doc.get("Circuit_attributes")->add("Voltage_sources_(V)",num_voltage_sources);
  }
  if (device_count.find('I') != device_count.end())
  {
    int num_current_sources = device_count['I'];
    doc.get("Circuit_attributes")->add("Current_sources_(I)",num_current_sources);
  }
  }

  int num_my_nnz = dae->A->local_nnz, sum_nnz = dae->A->local_nnz;
  int min_nnz = num_my_nnz, max_nnz = num_my_nnz;
  int num_my_rows = dae->A->local_rows.size();
  int min_rows = num_my_rows, max_rows = num_my_rows, sum_rows = num_my_rows;
  int num_my_overlap = dae->A->overlap_rows.size();
  int min_overlap = num_my_overlap, max_overlap = num_my_overlap, sum_overlap = num_my_overlap;

#ifdef HAVE_MPI
  MPI_Allreduce(&num_my_nnz,&sum_nnz,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_nnz,&min_nnz,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_nnz,&max_nnz,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_rows,&sum_rows,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_rows,&min_rows,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_rows,&max_rows,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_overlap,&sum_overlap,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_overlap,&min_overlap,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
  MPI_Allreduce(&num_my_overlap,&max_overlap,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
#endif

  doc.add("Matrix_attributes","");
  doc.get("Matrix_attributes")->add("Global_rows",total_unknowns);
  doc.get("Matrix_attributes")->add("Rows_per_proc_MIN",min_rows);
  doc.get("Matrix_attributes")->add("Rows_per_proc_MAX",max_rows);
  doc.get("Matrix_attributes")->add("Rows_per_proc_AVG",(double)sum_rows/p);
  doc.get("Matrix_attributes")->add("Global_overlap_rows",sum_overlap);
  doc.get("Matrix_attributes")->add("Rows_overlap_per_proc_MIN",min_overlap);
  doc.get("Matrix_attributes")->add("Rows_overlap_per_proc_MAX",max_overlap);
  doc.get("Matrix_attributes")->add("Rows_overlap_per_proc_AVG",(double)sum_overlap/p);
  doc.get("Matrix_attributes")->add("Global_NNZ",sum_nnz);
  doc.get("Matrix_attributes")->add("NNZ_per_proc_MIN",min_nnz);
  doc.get("Matrix_attributes")->add("NNZ_per_proc_MAX",max_nnz);
  doc.get("Matrix_attributes")->add("NNZ_per_proc_AVG",(double)sum_nnz/p);
        
  distributed_sparse_matrix* A = dae->A;
  distributed_sparse_matrix* B = dae->B;

  /*
  for (int proc = 0; proc < p; proc++)
  {
    if (pid == proc)
    {
      int num_my_sends = A->send_instructions.size();
      std::cout << "Processor : " << pid << " has " << num_my_sends << " sends: ";
      std::list<data_transfer_instruction*>::iterator it1;
      for (it1 = A->send_instructions.begin(); it1 != A->send_instructions.end(); it1++)
      {
        std::cout << " ( " << (*it1)->pid << " ) " << (*it1)->indices.size() << " ";
      }
      std::cout << std::endl;
      int num_my_recvs = A->recv_instructions.size();
      std::cout << "Processor : " << pid << " has " << num_my_recvs << " recvs: ";
      for (it1 = A->recv_instructions.begin(); it1 != A->recv_instructions.end(); it1++)
      {
        std::cout << " ( " << (*it1)->pid << " ) " << (*it1)->indices.size() << " ";
      }
      std::cout << std::endl;
    }
  }

  if (pid==0)
    std::cout << "A: " << std::endl;
  print_matrix( *A );

  if (pid==0)
    std::cout << "B: " << std::endl;
  print_matrix( *B );
  */

  distributed_vector init_RHS2 = evaluate_b(t_start,dae);

  // compute the initial condition if not specified by user

  tstart = mX_timer();
        
  distributed_vector x2( total_unknowns, p, pid, A->local_rows, A->overlap_rows, A->send_instructions, A->recv_instructions );

  if (pid==0)
    std::cout << "Initial condition specified: " << init_cond_specified << std::endl;

  if (!init_cond_specified)
  {
    mX_vector_utils::init_value( x2 , 0.0 );

    int curr_restarts = restarts; 
    gmres(A,init_RHS2,x2,tol,res,k,iters,curr_restarts);

    doc.add("DCOP Calculation","");
    doc.get("DCOP Calculation")->add("Init_cond_specified", false);
    doc.get("DCOP Calculation")->add("GMRES_tolerance",tol);
    doc.get("DCOP Calculation")->add("GMRES_subspace_dim",k);
    doc.get("DCOP Calculation")->add("GMRES_iterations",iters);
    doc.get("DCOP Calculation")->add("GMRES_restarts",curr_restarts);
    doc.get("DCOP Calculation")->add("GMRES_native_residual",res);
  }
  else 
  {
    doc.add("DCOP Calculation","");
    doc.get("DCOP Calculation")->add("Init_cond_specified", true);
  }
  tend = mX_timer() - tstart;
  doc.get("DCOP Calculation")->add("DCOP_calculation_time",tend);

  // from now you won't be needing any more Ax = b solves
    // but you will be needing many (A + B/t_step)x = b solves
    // compute the nonzero entry pattern for (A + B/t_step) right now
      // so you won't have to compute it at each time step
 
  tstart = mX_timer();
  distributed_sparse_matrix ApB( total_unknowns, p, pid );

  std::vector<distributed_sparse_matrix_entry*>::iterator it1;
  int row_idx = -1;

  // First insert A.
  for (it1 = A->row_headers.begin(); it1 != A->row_headers.end(); it1++)
  {
    row_idx++;
    distributed_sparse_matrix_entry* curr = *it1;

    while (curr)
    {
      int col_idx = curr->column;
      double value = (curr->value);
      distributed_sparse_matrix_add_to(&ApB,row_idx,col_idx,value);

      curr = curr->next_in_row;
    }
  }

  // Then insert B.
  row_idx = -1;
  for (it1 = B->row_headers.begin(); it1 != B->row_headers.end(); it1++)
  {
    row_idx++;
    distributed_sparse_matrix_entry* curr = *it1;

    while (curr)
    {
      int col_idx = curr->column;
      double value = (curr->value);
      distributed_sparse_matrix_add_to(&ApB,row_idx,col_idx,value);

      curr = curr->next_in_row;
    }
  }

  double matrix_setup_tend = mX_timer() - tstart;

  // even though you use a constant timestep, in practice that time step
    // is variable, so the combination of A + B/t_step will be computed
    // for each timestep to get the correct performance numbers. 

  tstart = mX_timer();
  double t = t_start + t_step;
  double total_res_load = 0.0;
  double total_jac_load = 0.0;
  double total_jac_assembly = 0.0;
  double inner_tstart = 0.0;

  int trans_steps = 0;

  while (t < t_stop)
  {
    trans_steps++;

    // --------------------------------------------------
    // new time point t => new value for b(t)
    // --------------------------------------------------

    inner_tstart = mX_timer();

    distributed_vector b = evaluate_b(t,dae);

    total_res_load += (mX_timer() - inner_tstart);

    // --------------------------------------------------
    // load matrices to assemble the Jacobian
    // NOTE:  this cannot be loaded directly into ApB
    //        because the devices have direct access to
    //        the memory in A and B to load the values.
    // --------------------------------------------------

    inner_tstart = mX_timer();

    load_matrices( dae );

    total_jac_load += (mX_timer() - inner_tstart);

    // --------------------------------------------------
    // assemble jacobian
    // --------------------------------------------------

    inner_tstart = mX_timer();
    init_value(ApB, 0.0);

    // first insert A.
    for (it1 = A->row_headers.begin(), row_idx=0; it1 != A->row_headers.end(); it1++, row_idx++)
    {
      distributed_sparse_matrix_entry* curr = *it1;

      while (curr)
      {
        int col_idx = curr->column;
        double value = (curr->value);
        distributed_sparse_matrix_add_to(&ApB,row_idx,col_idx,value);

        curr = curr->next_in_row;
      }
    }

    // then insert B.
    for (it1 = B->row_headers.begin(), row_idx=0; it1 != B->row_headers.end(); it1++, row_idx++)
    {
      distributed_sparse_matrix_entry* curr = *it1;

      while (curr)
      {
        int col_idx = curr->column;
        double value = (curr->value)/t_step;
        distributed_sparse_matrix_add_to(&ApB,row_idx,col_idx,value);
 
        curr = curr->next_in_row;
      }
    }
   
    total_jac_assembly += (mX_timer() - inner_tstart);

    // increment t
    t += t_step;
  }

  // Document the accrued time for the transient device loads.
  // No solves are being performed in transient.
  doc.add("Transient Calculation","");
  doc.get("Transient Calculation")->add("Number_of_time_steps", trans_steps);
  doc.get("Transient Calculation")->add("Time_start", t_start);
  doc.get("Transient Calculation")->add("Time_end", t_stop);
  doc.get("Transient Calculation")->add("Time_step", t_step);
  doc.get("Transient Calculation")->add("Matrix_setup_time",matrix_setup_tend);
  doc.get("Transient Calculation")->add("Residual_load_time",total_res_load);
  doc.get("Transient Calculation")->add("Jacobian_load_time",total_jac_load);
  doc.get("Transient Calculation")->add("Jacobian_assembly_time",total_jac_assembly);

  if (pid==0) { // Only PE 0 needs to compute and report timing results

    std::string yaml = doc.generateYAML();
    std::cout << yaml;
  }

  // Clean up
  mX_DAE_utils::destroy( dae );

#ifdef HAVE_MPI  
  MPI_Finalize();
#endif

  return 0;
}

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <string>
#include <cmath>


using namespace mX_source_utils;

// ------------------------------------------------------------------
// Implementations for the SINE class
mX_source_utils::SINE::SINE(double off, double amp, double f, double ph)
  : offset(off),
    amplitude(amp),
    freq(f),
    phase(ph)
{}

double mX_source_utils::SINE::output(double t) 
{
  double pi = 3.1415926535;
  return (offset + amplitude * sin(2*pi*freq*t + phase));
}

// ------------------------------------------------------------------
// Implementations for the FM class
mX_source_utils::FM::FM(double o, double a, double cf, double mi, double sf)
  : offset(o),
    amplitude(a),
    carrier_freq(cf),
    modulation_index(mi),
    signal_freq(sf)
{}

double mX_source_utils::FM::output(double t) 
{
  double pi = 3.1415926535;
  return (offset + amplitude * sin(2*pi*carrier_freq*t + modulation_index*sin(2*pi*signal_freq*t)));
}

// ------------------------------------------------------------------
// Implementations for the PWL class
mX_source_utils::PWL::PWL(std::vector<double> ts, std::vector<double> vals)
  : times(ts),
    values(vals)
{}

double mX_source_utils::PWL::output(double t) 
{
  if (times.size() == 1)
  {
    return values[0];
  }

  if (t <= times[0])
  {
    return values[0];
  }

  if (t >= times.back())
  {
    return values.back();
  }

  int start_index = 0;
  int end_index = times.size()-1;
  int mid_index = (start_index + end_index)/2;
			
  while(true)
  {
    if (end_index - start_index == 1)
    {
      break;
    }
    if (times[mid_index] > t)
    {
      end_index = mid_index;
      mid_index = (start_index + end_index)/2;
    }
    else
    {
      if (times[mid_index] < t)
      {
        start_index = mid_index;
        mid_index = (start_index + end_index)/2;
      }
      else
      {
        return values[mid_index];
      }
    }
  }

  return (values[start_index] + (values[end_index]-values[start_index])*((t - times[start_index])/(times[end_index] - times[start_index])));
}

// ------------------------------------------------------------------
// Implementations for the PULSE class

mX_source_utils::PULSE::PULSE(double d1, double d2, double td, double tr, double pw, double tf, double ts)
  : val1(d1),
    val2(d2),
    t_delay(td),
    t_rise(tr),
    pulse_width(pw),
    t_fall(tf),
    t_stop(ts)
{}

double mX_source_utils::PULSE::output(double t)
{
  double t_total = t_delay + t_rise + pulse_width + t_fall + t_stop;
  double n = floor(t/t_total);
  double t_disp = t - n*t_total;

  if ((t_disp >= 0) && (t_disp <= t_delay))
  {
    return val1;
  }				

  if ((t_disp >= t_delay) && (t_disp <= t_delay + t_rise))
  {
    return (val1 + ((val2-val1)/(t_rise))*(t_disp-t_delay));
  }

  if ((t_disp >= t_delay + t_rise) && (t_disp <= t_delay + t_rise + pulse_width))
  {
    return val2;
  }
			
  if ((t_disp >= t_delay + t_rise + pulse_width) && (t_disp <= t_delay + t_rise + pulse_width + t_fall))
  {
    return val2 + ((val1-val2)/(t_fall))*(t_disp-(t_delay + t_rise + pulse_width));
  }

  return val1;
}

// ------------------------------------------------------------------
// Implementation to parse the source type

mX_source* mX_source_utils::parse_source(std::istringstream& input_str)
{
	// ok, so you know you have a source in the box
		// but you don't know what kind of beast it is
		// this function will look at the box and return the correct source object for you

	std::string src_type;
	input_str >> src_type;

	if (src_type == "DC")
	{
		double val;
		input_str >> val;
		return new DC(val);
	}

	if (src_type == "SIN")
	{
		double offset,amp,f,ph;
		input_str >> offset >> amp >> f >> ph;
		return new SINE(offset,amp,f,ph);
	}

	if (src_type == "FM")
	{
		double offset,amp,cf,mi,sf;
		input_str >> offset >> amp >> cf >> mi >> sf;
		return new FM(offset,amp,cf,mi,sf);
	}

	if (src_type == "PWL")
	{
		int pwl_size;
		std::vector<double> times;
		std::vector<double> values;

		double t,v;

		input_str >> pwl_size;

		for (int i = 0; i < pwl_size; i++)
		{
			input_str >> t >> v;
			times.push_back(t);
			values.push_back(v);
		}

		return new PWL(times,values);
	}

	if (src_type == "PULSE")
	{
		double v1,v2,td,tr,pw,tf,ts;
		input_str >> v1 >> v2 >> td >> tr >> pw >> tf >> ts;
		return new PULSE(v1,v2,td,tr,pw,tf,ts);
	}

	return 0;
}

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#ifdef HAVE_MPI
#include <mpi.h> // If this routine is compiled with -DHAVE_MPI
                 // then include mpi.h
#endif

#include <algorithm>
#include <iostream>

using namespace mX_source_utils;
using namespace mX_matrix_utils;
using namespace mX_DAE_utils;

distributed_vector mX_DAE_utils::evaluate_b(double t, mX_DAE* dae)
{
  distributed_vector global_result( dae->A->n, dae->A->p, dae->A->my_pid,
                                    dae->A->local_rows, dae->A->overlap_rows,
                                    dae->A->send_instructions, dae->A->recv_instructions );

  std::vector<mX_DAE_RHS_entry*>::iterator it2;
  std::map<int,int>::iterator row_it;
  for (int i = 0; i < dae->b.size(); i++)
  {
    if ( dae->b[i] )
    {
      double sum = 0.0;
      std::list<mX_scaled_source*>::iterator it3 = dae->b[i]->scaled_src_list.begin();

      for( ; it3 != dae->b[i]->scaled_src_list.end(); it3++)
      {
        mX_source* src = (*it3)->src;
        sum += src->output(t * (*it3)->scale);
      }

      row_it = global_result.row_to_idx.find( i );
      if (row_it != global_result.row_to_idx.end())
      {
        global_result.values[row_it->second] = sum;
      }
    }
  }

  mX_vector_utils::assemble_vector( global_result );

  return global_result;
}

std::vector<double> mX_DAE_utils::evaluate_b_old(double t, mX_DAE* dae)
{
  // given a linear DAE "A x + B x_dot = b(t)"
  // and a particular time point t
  // this function computes and returns the vector b at that time point t

  std::vector<mX_DAE_RHS_entry*>::iterator it2;
  std::vector<double> local_result, global_result;

  for (it2 = dae->b.begin(); it2 != dae->b.end(); it2++)
  {
    if (!(*it2))
    {
      local_result.push_back(0.0);
    }
    else
    {
      double sum = (double)(0);
      std::list<mX_scaled_source*>::iterator it3;

      for(it3 = (*it2)->scaled_src_list.begin(); it3 != (*it2)->scaled_src_list.end(); it3++)
      {
	mX_source* src = (*it3)->src;
	sum += src->output((double)(t)) * ((*it3)->scale);
      }

      local_result.push_back(sum);
    }
  }

#ifdef HAVE_MPI
  global_result.resize( dae->b.size(), 0.0 );
  MPI_Allreduce(&local_result[0],&global_result[0],dae->b.size(),MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
  return global_result;
#else
  return local_result;
#endif

}

void mX_DAE_utils::destroy_RHS(mX_DAE_RHS_entry* entry)
{
  if (entry)
  {
    // delete send_instructions
    while (!entry->scaled_src_list.empty())
    {
      mX_scaled_source* curr = entry->scaled_src_list.front();
      if (curr) delete curr->src; delete curr; curr=0;
      entry->scaled_src_list.pop_front();
    }
    delete entry; entry=0;
  }
}

void mX_DAE_utils::destroy(mX_DAE* dae)
{
  // Destroy A
  destroy_matrix( dae->A );

  // Destroy B
  destroy_matrix( dae->B );

  // Destroy b
  std::vector<mX_DAE_RHS_entry*>::iterator b_it = dae->b.begin();
  std::vector<mX_DAE_RHS_entry*>::iterator b_end = dae->b.end();
  for ( ; b_it != b_end; b_it++ )
  {
    destroy_RHS( *b_it );
  }
  dae->b.clear();

  // Destroy devices
  std::map<char, mX_device*>::iterator d_it = dae->devices.begin();
  std::map<char, mX_device*>::iterator d_end = dae->devices.end();
  for ( ; d_it != d_end; d_it++ )
  {
    delete d_it->second;
  }
  dae->devices.clear();

  delete dae; dae=0;
}

void mX_DAE_utils::load_matrices(mX_DAE* dae)
{
  // Initialize values to zero.
  init_value( *(dae->A), 0.0 );
  init_value( *(dae->B), 0.0 );

  std::map<char, mX_device*>::iterator d_it = dae->devices.begin();
  std::map<char, mX_device*>::iterator d_end = dae->devices.end();
  for ( ; d_it != d_end; d_it++ )
  {
    if (d_it->second)
      d_it->second->load_matrices();
  }
}


//@HEADER
// ************************************************************************
//
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2018

#include <iostream>

#ifndef __MX_LIN_DEVICE_DEFS_H__
#define __MX_LIN_DEVICE_DEFS_H__

//@HEADER
// ************************************************************************
//
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2018



#include <sstream>
#include <string>
#include <vector>

using namespace mX_vector_utils;
using namespace mX_matrix_utils;

namespace mX_DAE_utils
{
  struct mX_DAE;
}


namespace mX_device_utils
{

class mX_resistor : public mX_device
{
        public:

        mX_resistor();
        virtual ~mX_resistor() {}

        int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae );

        void load_vector( distributed_vector& vec );
        void load_matrices();

        int modelCost() { return 1; }

        private:

        std::map<std::string, int> device_map_;
        std::vector<std::vector<int> > device_nodes_;
        std::vector<double> R_;
};

class mX_inductor : public mX_device
{
        public:

        mX_inductor(); 
        virtual ~mX_inductor() {}

        int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae );

        void load_vector( distributed_vector& vec ); 
        void load_matrices();

        int modelCost() { return 1; }

        private:

        std::map<std::string, int> device_map_;
        std::vector<std::vector<int> > device_nodes_;
        std::vector<double> L_;
};

class mX_capacitor : public mX_device
{
        public:

        mX_capacitor();
        virtual ~mX_capacitor() {}

        int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae );

        void load_vector( distributed_vector& vec ); 
        void load_matrices();

        int modelCost() { return 1; }

        private:

        std::map<std::string, int> device_map_;
        std::vector<std::vector<int> > device_nodes_;
        std::vector<double> C_;
};

class mX_vsrc: public mX_device
{
        public:

        mX_vsrc();
        virtual ~mX_vsrc() {}

        int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae );

        void load_vector( distributed_vector& vec );
        void load_matrices();

        int modelCost() { return 1; }

        private:

};

class mX_isrc: public mX_device
{
        public:

        mX_isrc();
        virtual ~mX_isrc() {}

        int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae );

        void load_vector( distributed_vector& vec );
        void load_matrices();

        int modelCost() { return 1; }

        private:

};

}

#endif // __MX_DEVICE_DEFS_H__

#ifndef __MX_NL_DEVICE_DEFS_H__
#define __MX_NL_DEVICE_DEFS_H__

//@HEADER
// ************************************************************************
//
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2018



#include <sstream>
#include <string>
#include <vector>

using namespace mX_vector_utils;
using namespace mX_matrix_utils;

namespace mX_DAE_utils
{
  struct mX_DAE;
}


namespace mX_device_utils
{

class mX_diode: public mX_device
{
        public:

        mX_diode();
        virtual ~mX_diode() {}

        int add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae );

        void load_vector( distributed_vector& vec );
        void load_matrices();

        int modelCost() { return 50; }

        private:

};

std::vector<double> diode_l1(std::vector<double> &x);

}

#endif // __MX_NL_DEVICE_DEFS_H__



using namespace mX_DAE_utils;

namespace mX_device_utils
{

//
// Create device container
//
mX_device* create_device( char device_label )
{
  mX_device* new_device = 0;

  switch( device_label )
  {
    case 'R':
    {    
      new_device = new mX_resistor();
      break;
    }
    case 'C':
    {
      new_device = new mX_capacitor();
      break;
    }
    case 'L':
    {
      new_device = new mX_inductor();
      break;
    }
    case 'V':
    {
      new_device = new mX_vsrc();
      break;
    }
    case 'I':
    {
      new_device = new mX_isrc();
      break;
    }
    case 'D':
    {
      new_device = new mX_diode();
      break;
    }
    default:
    {
      std::cout << "Device type " << device_label << " is not recognized!" << std::endl;
      break;
    }
  }

  return new_device;
} 

void add_device_nodes( char device_label, std::istringstream& input_str, std::vector<int>& node_list )
{
  // Each device has at least 2 nodes, some have more.
  // As we add device models, those that have more will be inserted here.
  int node1, node2;
  input_str >> node1;
  input_str >> node2;
  node_list.push_back( node1 );
  node_list.push_back( node2 );
}


int num_internal_nodes( char device_label, std::istringstream& input_str )
{
  int num_nodes = 0;

  // Add internal nodes for KCL based on certain device models.
  switch( device_label )
  {
    case 'V':
    {
      num_nodes = 1;
    }
    break;

    case 'L':
    {
      num_nodes = 1;
    }
    break;
   
    case 'D':
    {
      num_nodes = 1;
    }
  }

  return num_nodes;
}




}



//@HEADER
// ************************************************************************
//
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2018

#include <iostream>




using namespace mX_source_utils;
using namespace mX_DAE_utils;

namespace mX_device_utils
{

//
// class mX_resistor 
//
mX_resistor::mX_resistor()
: mX_device('R')
{}

int mX_resistor::add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae )
{
  int node1, node2;
  input_str >> node1;
  input_str >> node2;

  double rvalue;
  input_str >> rvalue;

  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node1-1,node1-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node2-1,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node1-1,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node2-1,node1-1) );

  R_.push_back( rvalue ); 

  return 0;
}

void mX_resistor::load_vector( distributed_vector& vec )
{
}

void mX_resistor::load_matrices()
{
  std::vector<double>::iterator r_it = R_.begin();
  std::vector<double>::iterator r_end = R_.end();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_it =  device_entries_.begin();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_end =  device_entries_.end();
   
  for ( ; r_it != r_end && d_it != d_end; r_it++ )
  {
    double rvalue = 1.0/(*r_it);
 
    (*d_it)->value += rvalue; d_it++;
    (*d_it)->value += rvalue; d_it++;
    (*d_it)->value -= rvalue; d_it++;
    (*d_it)->value -= rvalue; d_it++;
  }
}

//
// class mX_inductor
//
mX_inductor::mX_inductor()
: mX_device('L')
{}

int mX_inductor::add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae )
{
  int node1, node2;
  input_str >> node1;
  input_str >> node2;
  
  double lvalue;
  input_str >> lvalue;
  
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,node1-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,extra_nodes_ptr,extra_nodes_ptr) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node1-1,extra_nodes_ptr) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node2-1,extra_nodes_ptr) );

  L_.push_back( lvalue );

  return 1;
}

void mX_inductor::load_vector( distributed_vector& vec ) 
{
}

void mX_inductor::load_matrices() 
{
  std::vector<double>::iterator l_it = L_.begin();
  std::vector<double>::iterator l_end = L_.end();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_it =  device_entries_.begin();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_end =  device_entries_.end();

  for ( ; l_it != l_end && d_it != d_end; l_it++ )
  {
    (*d_it)->value += 1.0; d_it++;
    (*d_it)->value -= 1.0; d_it++;
    (*d_it)->value -= *l_it; d_it++;
    (*d_it)->value += 1.0; d_it++;
    (*d_it)->value -= 1.0; d_it++;
  }
}

//
// class mX_capacitor 
//
mX_capacitor::mX_capacitor()
: mX_device('C')
{}

int mX_capacitor::add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae )
{
  int node1, node2;
  input_str >> node1;
  input_str >> node2;

  double cvalue;
  input_str >> cvalue;

  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,node1-1,node1-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,node2-1,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,node1-1,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,node2-1,node1-1) );
  
  C_.push_back( cvalue );

  return 0;
}

void mX_capacitor::load_vector( distributed_vector& vec ) 
{
}

void mX_capacitor::load_matrices()
{
  std::vector<double>::iterator c_it = C_.begin();
  std::vector<double>::iterator c_end = C_.end();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_it =  device_entries_.begin();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_end =  device_entries_.end();
   
  for ( ; c_it != c_end && d_it != d_end; c_it++ )
  { 
    (*d_it)->value += *c_it; d_it++;
    (*d_it)->value += *c_it; d_it++;
    (*d_it)->value -= *c_it; d_it++;
    (*d_it)->value -= *c_it; d_it++;
  }
}

//
// class mX_vsrc
//
mX_vsrc::mX_vsrc()
: mX_device('V')
{}

int mX_vsrc::add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae )
{ 
  int node1, node2;
  input_str >> node1;
  input_str >> node2;

  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,node1-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node1-1,extra_nodes_ptr) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node2-1,extra_nodes_ptr) );

  mX_source* src = parse_source(input_str);

  mX_scaled_source* scaled_src = new mX_scaled_source();
  scaled_src->src = src;
  scaled_src->scale = (double)(1);

  if(!(dae->b[extra_nodes_ptr]))
  {
    dae->b[extra_nodes_ptr] = new mX_DAE_RHS_entry();
  }

  (dae->b[extra_nodes_ptr])->scaled_src_list.push_back(scaled_src);

  return 1;
}

void mX_vsrc::load_vector( distributed_vector& vec )
{
}

void mX_vsrc::load_matrices()
{
  std::vector<distributed_sparse_matrix_entry* >::iterator d_it =  device_entries_.begin();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_end =  device_entries_.end();

  for ( ; d_it != d_end; )
  {
    (*d_it)->value += 1.0; d_it++;
    (*d_it)->value -= 1.0; d_it++;
    (*d_it)->value -= 1.0; d_it++;
    (*d_it)->value += 1.0; d_it++;
  }
}

//
// class mX_isrc
//
mX_isrc::mX_isrc()
: mX_device('I')
{}

int mX_isrc::add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae )
{ 
  int node1, node2;
  input_str >> node1;
  input_str >> node2;

  if (node1)
  {
    mX_source* src = parse_source(input_str);

    mX_scaled_source* scaled_src = new mX_scaled_source();
    scaled_src->src = src;
    scaled_src->scale = (double)(1);

    if(!(dae->b[node1-1]))
    {
      dae->b[node1-1] = new mX_DAE_RHS_entry();
    }

    (dae->b[node1-1])->scaled_src_list.push_back(scaled_src);
  }
  if (node2)
  {
    mX_source* src = parse_source(input_str);

    mX_scaled_source* scaled_src = new mX_scaled_source();
    scaled_src->src = src;
    scaled_src->scale = (double)(-1);

    if(!(dae->b[node2-1]))
    {
      dae->b[node2-1] = new mX_DAE_RHS_entry();
    }

    (dae->b[node2-1])->scaled_src_list.push_back(scaled_src);
  }

  return 0;
}

void mX_isrc::load_vector( distributed_vector& vec )
{
}

void mX_isrc::load_matrices()
{
}


}



//@HEADER
// ************************************************************************
//
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : Sept 2018


#include <cmath>
#include <vector>

using namespace mX_DAE_utils;

namespace mX_device_utils
{

mX_diode::mX_diode()
: mX_device('D')
{}

int mX_diode::add_device( std::istringstream& input_str, int extra_nodes_ptr, mX_DAE_utils::mX_DAE* dae )
{ 
  int node1, node2;
  input_str >> node1;
  input_str >> node2;
  
  // Don't process any more values, there is an independent function to evaluate.             
  //double dvalue;
  //input_str >> dvalue;
    
  // Insert A entries first. 
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node1-1,node1-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node1-1,extra_nodes_ptr) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node2-1,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,node2-1,extra_nodes_ptr) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,node1-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->A,extra_nodes_ptr,extra_nodes_ptr) );
  
  // Then insert B entres.
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,node2-1,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,node2-1,extra_nodes_ptr) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,extra_nodes_ptr,node2-1) );
  device_entries_.push_back( distributed_sparse_matrix_insert(dae->B,extra_nodes_ptr,extra_nodes_ptr) );
  
  return 1;
}

void mX_diode::load_vector( distributed_vector& vec )
{
}

void mX_diode::load_matrices()
{ 
  std::vector<distributed_sparse_matrix_entry* >::iterator d_it =  device_entries_.begin();
  std::vector<distributed_sparse_matrix_entry* >::iterator d_end =  device_entries_.end();
  
  for ( ; d_it != d_end; )
  { 
    // Evaluate model
    std::vector<double> input_vals( 3, std::rand() );
    std::vector<double> model_vals = diode_l1( input_vals );
    
    double Gspr = model_vals[0];
    double Gd = model_vals[1];
    double Cd = model_vals[2];
    
    // A-matrix
    (*d_it)->value += Gspr; d_it++;
    (*d_it)->value -= Gspr; d_it++;
    (*d_it)->value += Gd; d_it++;
    (*d_it)->value -= Gd; d_it++;
    (*d_it)->value -= Gspr; d_it++;
    (*d_it)->value -= Gd; d_it++;
    (*d_it)->value += Gspr + Gd; d_it++;
    
    // B-matrix
    (*d_it)->value += Cd; d_it++;
    (*d_it)->value -= Cd; d_it++;
    (*d_it)->value -= Cd; d_it++;
    (*d_it)->value += Cd; d_it++;
  }
}

std::vector<double> diode_l1(std::vector<double> &x)
{
    double z0 = x[0];
    double z1 = x[1];
    double z2 = x[2];

    double z3 = +z0;
    double z4 = z2 * z1;
    double z5 = std::abs(z4);
    double z6 = std::cos(z5);
    double z7 = std::sin(z3);
    double z8 = z6 + z7;
    double z9 = std::cbrt(z8);
    double z10 = std::tanh(z9);
    double z11 = z10 * z8;
    double z12 = std::tanh(z11);
    double z13 = std::cbrt(z12);
    double z14 = z13 * z10;
    double z15 = z14 - z0;
    double z16 = z15 + z7;
    double z17 = z16 + z14;
    double z18 = std::pow(z17, 2.0);
    double z19 = std::atan(z18);
    double z20 = -z19;
    double z21 = std::sin(z20);
    double z22 = std::cbrt(z21);
    double z23 = std::sin(z22);
    double z24 = z23 + z14;
    double z25 = std::cos(z24);
    double z26 = z25 - z11;
    double z27 = std::atan(z26);
    double z28 = std::pow(z27, 3.0);
    double z29 = z28 + z28;
    double z30 = z29 * z17;
    double z31 = std::atan(z30);
    double z32 = std::tanh(z31);
    double z33 = z32 + z2;
    double z34 = -z33;
    double z35 = z34 - z12;
    double z36 = std::atan(z35);
    double z37 = std::abs(z36);
    double z38 = std::abs(z37);
    double z39 = z38 + z23;
    double z40 = std::cos(z39);
    double z41 = std::cos(z40);
    double z42 = +z41;
    double z43 = z42 - z6;
    double z44 = std::sin(z43);
    double z45 = +z44;
    double z46 = std::pow(z45, 3.0);
    double z47 = z46 * z28;
    double z48 = std::cbrt(z47);
    double z49 = z48 + z30;
    double z50 = std::cos(z49);
    double z51 = std::atan(z50);
    double z52 = std::tanh(z51);

    double z53 = z52;
    double z54 = z52;
    double z55 = z11;

    std::vector<double> y;
    y.push_back(z53);
    y.push_back(z54);
    y.push_back(z55);

    return y;
}

}

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <string>
#include <set>
#include <vector>
#include <cstdlib>
#include <fstream>

#include <cassert>

using namespace mX_parms_utils;

void mX_parms_utils::parse_command_line(int argc, std::vector<std::string> &argv, std::string &ckt_filename, double &t_start, double &t_step, double &t_stop, double &tol, int &k, int &max_restarts,
                                        std::vector<double> &init_cond, std::string &parms_file, std::set<int> &specified_parms, int p, int pid)
{
  // take a command-line-ish argc and argv
  // parse it to get the parms specified there
      // change those parms if they haven't been already specified earlier

  int i = 1;

  while (i < argc)
  {
    // got to read in a new parameter and its value

    std::string parm_name = argv[i];
    i++;

    int first_non_minus_index = parm_name.find_first_not_of('-');
    assert(first_non_minus_index != 0);
    parm_name = parm_name.substr(first_non_minus_index);

    if ((parm_name == "c") || (parm_name == "circuit"))
      {
      if (specified_parms.find(CKT_FILENAME) == specified_parms.end())
      {
        specified_parms.insert(CKT_FILENAME);
        ckt_filename = argv[i];
      }
      
      i++;
    }

    if ((parm_name == "ti") || (parm_name == "t_start") || (parm_name == "tstart"))
    {
      if (specified_parms.find(T_START) == specified_parms.end())
      {
        specified_parms.insert(T_START);
        t_start = atof(argv[i].data());
      }
      
      i++;
    }

    if ((parm_name == "tf") || (parm_name == "t_stop") || (parm_name == "tstop"))
    {
      if (specified_parms.find(T_STOP) == specified_parms.end())
      {
        specified_parms.insert(T_STOP);
        t_stop = atof(argv[i].data());
      }
      
      i++;
    }

    if ((parm_name == "h") || (parm_name == "t_step") || (parm_name == "tstep"))
    {
      if (specified_parms.find(T_STEP) == specified_parms.end())
      {
        specified_parms.insert(T_STEP);
        t_step = atof(argv[i].data());
      }
      
      i++;
    }

    if ((parm_name == "tol") || (parm_name == "tolerance"))
    {
      if (specified_parms.find(TOL) == specified_parms.end())
      {
        specified_parms.insert(TOL);
        tol = atof(argv[i].data());
      }
      
      i++;
    }

    if ((parm_name == "k"))
    {
      if (specified_parms.find(K) == specified_parms.end())
      {
        specified_parms.insert(K);
        k = atoi(argv[i].data());
      }

      i++;
    }

    if ((parm_name == "max_restarts"))
    {
      if (specified_parms.find(MAX_RESTART) == specified_parms.end())
      {
        specified_parms.insert(MAX_RESTART);
        max_restarts = atoi(argv[i].data());
      }

      i++;
    }

    if ((parm_name == "pf") || (parm_name == "paramsfile") || (parm_name == "params_file"))
    {
      if (specified_parms.find(PARMS_FILE) == specified_parms.end())
      {
        specified_parms.insert(PARMS_FILE);
        parms_file = argv[i];
      }
      
      i++;
    }

    if (parm_name == "prev")
    {
      specified_parms.insert(PREV);
    }

    if ((parm_name == "i") || (parm_name == "init") || (parm_name == "initcond") || (parm_name == "init_cond") || (parm_name == "x0"))
    {
      std::vector<std::string> init;
      int n = 0;

      while (i < argc)
      {
        std::string next_arg = argv[i];

        if (next_arg[0] == '-')
        {
          break;
        }

        init.push_back(next_arg);
        i++; n++;
      }

      if (specified_parms.find(INIT_COND) == specified_parms.end())
      {
        specified_parms.insert(INIT_COND);
        
        int start_row = (n/p)*(pid) + ((pid < n%p) ? pid : n%p);
        int end_row = start_row + (n/p) - 1 + ((pid < n%p) ? 1 : 0);
  
        for (int j = start_row; j <= end_row; j++)
        {
          if (j-start_row < init_cond.size())
          {
            init_cond[j-start_row] = atof(init[j].data());
          }
  
          else
          {
            init_cond.push_back(atof(init[j].data()));
          }
        }
      }
    }
  }
}

std::vector<std::string> mX_parms_utils::get_command_line_equivalent_from_file(std::string &filename)
{
  // read a file that has parameters stored in it
    // construct a command-line-ish sequence from those parameters

  std::ifstream infile;
  infile.open(filename.data());

  if (infile.fail())
  {
    std::vector<std::string> v;
    return v;
  }

  std::vector<std::string> command_line_equivalent;
  command_line_equivalent.push_back("ignore");

  while (!infile.eof())
  {
    std::string curr_line;
    getline(infile,curr_line);

    if ((curr_line.length() == 0) || (curr_line[0] == '%'))
    {
      continue;
    }

    std::string LHS,RHS;

    int equals_idx = curr_line.find_first_of('=');
    LHS = curr_line.substr(0,equals_idx);
    RHS = curr_line.substr(equals_idx+1);

    int first_significant_index = LHS.find_first_not_of(" \t");
    int last_significant_index = LHS.find_last_not_of(" \t");
    LHS = LHS.substr(first_significant_index, last_significant_index-first_significant_index+1);

    first_significant_index = RHS.find_first_not_of(" \t");
    last_significant_index = RHS.find_last_not_of(" \t");
    RHS = RHS.substr(first_significant_index, last_significant_index-first_significant_index+1);

    command_line_equivalent.push_back("--" + LHS);
    command_line_equivalent.push_back(RHS);
  }

  infile.close();

  return command_line_equivalent;
}

void mX_parms_utils::get_parms(int argc, char* argv[], std::string &ckt_filename, double &t_start, double &t_step, double &t_stop, double &tol, int &k, int &max_restarts,
                               std::vector<double> &init_cond, bool &init_cond_specified, int p, int pid)
{
  // get all the required simulation parameters
    // first go through command line options, they get first priority
    // after looking through the command line
      // if the command line options specify a file
        // open the file and read the parameters stored there
    // then, if the command line options specify --prev
      // read parameters from last_used_params.txt
    // then read remaining parameters from default_params.txt

  std::set<int> specified_parms;
  std::string parms_file;

  std::vector<std::string> argv_strings;

  for (int i = 0; i < argc; i++)
  {
    argv_strings.push_back((std::string)(argv[i]));
  }

  parse_command_line(argc, argv_strings, ckt_filename, t_start, t_step, t_stop, tol, k, max_restarts, init_cond, parms_file, specified_parms, p, pid);

  // scanned all command line parameters
    // now scan any files that are necessary

  if (specified_parms.find(PARMS_FILE) != specified_parms.end())
  {
    argv_strings = get_command_line_equivalent_from_file(parms_file);
    parse_command_line(argv_strings.size(), argv_strings, ckt_filename, t_start, t_step, t_stop, tol, k, max_restarts, init_cond, parms_file, specified_parms, p, pid);
  }

  if (specified_parms.find(PREV) != specified_parms.end())
  {
    std::string filename = "last_used_params.txt";
    argv_strings = get_command_line_equivalent_from_file(filename);
    parse_command_line(argv_strings.size(), argv_strings, ckt_filename, t_start, t_step, t_stop, tol, k, max_restarts, init_cond, parms_file, specified_parms, p, pid);
  }

  std::string filename = "default_params.txt";
  argv_strings = get_command_line_equivalent_from_file(filename);
  parse_command_line(argv_strings.size(), argv_strings, ckt_filename, t_start, t_step, t_stop, tol, k, max_restarts, init_cond, parms_file, specified_parms, p, pid);
  
  // by now all parms must have been obtained

  assert(specified_parms.find(CKT_FILENAME) != specified_parms.end());
  assert(specified_parms.find(T_START) != specified_parms.end());
  assert(specified_parms.find(T_STEP) != specified_parms.end());
  assert(specified_parms.find(T_STOP) != specified_parms.end());
  assert(specified_parms.find(TOL) != specified_parms.end());
  assert(specified_parms.find(K) != specified_parms.end());
  assert(specified_parms.find(MAX_RESTART) != specified_parms.end());

  init_cond_specified = (specified_parms.find(INIT_COND) != specified_parms.end());


}

//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#ifdef HAVE_MPI
#include <mpi.h> // If this routine is compiled with -DHAVE_MPI
                 // then include mpi.h
#endif




#include <map>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>

using namespace mX_comm_utils;
using namespace mX_matrix_utils;
using namespace mX_vector_utils;
using namespace mX_DAE_utils;

distributed_sparse_matrix::distributed_sparse_matrix()
  : start_row(0), end_row(0), local_nnz(0)
{}

distributed_sparse_matrix::distributed_sparse_matrix(int total_unknowns, int num_procs, int pid)
  : start_row(0),
    end_row(0), 
    local_nnz(0),
    n(total_unknowns),
    p(num_procs),
    my_pid(pid)
{
  // Initialize row_headers so that entries can be inserted.
  for (int i = 0; i < total_unknowns; i++)
  {
    distributed_sparse_matrix_entry* null_ptr_1 = 0;
    row_headers.push_back(null_ptr_1);
  }
}

distributed_sparse_matrix_entry* mX_matrix_utils::distributed_sparse_matrix_insert(distributed_sparse_matrix* M, int row_idx, int col_idx)
{
  distributed_sparse_matrix_entry* ret = &M->ground_node;

  // Inserts row_idx,col_idx,val into M.
  if ((row_idx < 0) || (col_idx < 0))
  {
    // check for negative indices
      // useful in cases where you want to ignore the ground
        // because the ground is not really a node

    return ret;
  }

  bool inserted = false;
  bool found = false;

  distributed_sparse_matrix_entry* prev = 0;
  distributed_sparse_matrix_entry* curr = M->row_headers[row_idx];

  while (curr && !found)
  {
    if (curr->column < col_idx)
    {
      prev = curr;
      curr = curr->next_in_row;
    }
    else
    {
      if (curr->column == col_idx)
      {
        ret = curr;
        found = true;
      }
      else
      {
        // The entry should go here, but it's not less than or equal to, so the col_idx must be >.
        break;
      }
    }
  }

  if (!found)
  {
    ret = new distributed_sparse_matrix_entry(col_idx, 0.0, curr);

    if (prev)
    {
      prev->next_in_row = ret;
    }
    else
    {
      M->row_headers[row_idx] = ret;
    }

    inserted = true;
  }

  // The number of local nonzeros has gone up only if this is a new entry.
  if (inserted)
    M->local_nnz++;

  return ret;
}

void mX_matrix_utils::distributed_sparse_matrix_insert(distributed_sparse_matrix* M, int row_idx, int col_idx, double val)
{
  // Find the entry or make a new entry using row and column index.
  distributed_sparse_matrix_entry* entry = distributed_sparse_matrix_insert(M, row_idx, col_idx);

  // Update the value.
  if (entry)
    entry->value += val;
}

void mX_matrix_utils::distributed_sparse_matrix_finish(distributed_sparse_matrix* A, distributed_sparse_matrix* B,
                                                       const std::vector<mX_DAE_RHS_entry*>& b)
{
  // MPI_MIN is used to assign processors to shared nodes, so put the 
  // fill value as the number of processors for nodes not required by this processor.
  // 
  // The sends and recvs are set up using the nonzero pattern of both A, B, and b.
  // This ensures we can form the circuit Jacobian and perform matrix-vector products.
  //
  int fill_value = A->p; 

  std::vector<int> local_nnz_rows(A->n,fill_value), global_nnz_rows(A->n,0);
  std::vector<int> local_nnz_cols;

  for (int i=0; i<A->n; i++)
  {
    distributed_sparse_matrix_entry* curr = A->row_headers[i];
    if (curr)
      local_nnz_rows[i] = A->my_pid;
    while (curr)
    {
      local_nnz_cols.push_back(curr->column);
      curr = curr->next_in_row;
    }

    curr = B->row_headers[i];
    if (curr)
      local_nnz_rows[i] = B->my_pid;
    while (curr)
    {
      local_nnz_cols.push_back(curr->column);
      curr = curr->next_in_row;
    } 

    mX_DAE_RHS_entry* curr_b = b[i];
    if (curr_b)
    {
      local_nnz_rows[i] = A->my_pid; 
      local_nnz_cols.push_back(i);
    }
  } 

  // Order the columns needed for matrix-vector products.
  std::sort( local_nnz_cols.begin(), local_nnz_cols.end() );
  local_nnz_cols.erase( std::unique( local_nnz_cols.begin(), local_nnz_cols.end() ), local_nnz_cols.end() ); 
 
#ifdef HAVE_MPI
  MPI_Allreduce(&local_nnz_rows[0],&global_nnz_rows[0],A->n,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
#endif

  for (int i=0; i<A->n; i++)
  {
    // Determine who owns the nodes that are shared
    if (global_nnz_rows[i]==A->my_pid)
    {
      A->local_rows.push_back(i);
      //std::cout << "Processor " << A->my_pid << " owns row " << i << std::endl;

      std::vector<int>::iterator it = std::find( local_nnz_cols.begin(), local_nnz_cols.end(), i );
      if ( it != local_nnz_cols.end() )
      {
        local_nnz_cols.erase( it );
      }
    } 
    else if (local_nnz_rows[i]==A->my_pid)
    {
      // This processor will need to send the result of it's matrix-vector product to global_nnz_rows[i].
      // Since there is already a send_instructions for the column, we'll call this a recv_instructions.
      // For linear circuits, matrices are symmetric, so recv_instructions will be the transpose of send.
      bool recv_instruction_posted = false;
      int pid_to_recv_info = global_nnz_rows[i];

      std::list<data_transfer_instruction*>::iterator it1 = A->recv_instructions.begin();

      for (; it1 != A->recv_instructions.end(); it1++)
      {
        if ((*it1)->pid == pid_to_recv_info)
        {
          std::list<int>::iterator it2 = std::find( (*it1)->indices.begin(), (*it1)->indices.end(), i );
          if (it2 == (*it1)->indices.end())
          {
            (*it1)->indices.push_back(i);
          }
          recv_instruction_posted = true;
        }
      }

      if (!recv_instruction_posted)
      {
        data_transfer_instruction* dti_ptr_1 = new data_transfer_instruction(pid_to_recv_info);
        dti_ptr_1->indices.push_back(i);

        A->recv_instructions.push_back(dti_ptr_1);
      }
    }
  }

  for (std::vector<int>::iterator it = local_nnz_cols.begin(); it != local_nnz_cols.end(); it++)
  {
    //std::cout << "Processor " << A->my_pid << " has overlap row " << *it << std::endl;
  }

  // Set the local and ghost rows with both A and B.
  A->overlap_rows = local_nnz_cols;
  B->local_rows = A->local_rows;
  B->overlap_rows = local_nnz_cols;

  std::vector<int> local_overlap( A->p ), global_overlap( A->p );
  local_overlap[ A->my_pid ] = local_nnz_cols.size();

#ifdef HAVE_MPI
  MPI_Allreduce(&local_overlap[0],&global_overlap[0],A->p,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
#endif

  int total_overlap = std::accumulate( global_overlap.begin(), global_overlap.end(), 0 );

  int ptr = 0;
  std::vector<int> local_recv_rows( 2*total_overlap ), global_recv_rows( 2*total_overlap );
  for (int proc = 0; proc < A->p; proc++)
  {
    if (proc == A->my_pid)
    {
      for (std::vector<int>::const_iterator it = local_nnz_cols.begin(); it != local_nnz_cols.end(); it++)
      {
        local_recv_rows[ ptr ] = proc;
        local_recv_rows[ ptr+1 ] = *it;
        ptr += 2;
      }
      break;
    }
    ptr += 2*global_overlap[proc]; 
  }
  
#ifdef HAVE_MPI
  MPI_Allreduce(&local_recv_rows[0],&global_recv_rows[0],2*total_overlap,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
#endif

  for (int i=0; i<total_overlap; i++)
  {
    int pid_to_send_info = global_recv_rows[2*i];
    int row = global_recv_rows[2*i+1];

    if (pid_to_send_info != A->my_pid)
    {
      bool send_instruction_posted = false;
      std::vector<int>::const_iterator it = std::find( A->local_rows.begin(), A->local_rows.end(), row );
      if (it != A->local_rows.end())
      {
        //std::cout << "Processor " << A->my_pid << " needs to send row " << row << " to processor " << pid_to_send_info << std::endl;

        std::list<data_transfer_instruction*>::iterator it1 = A->send_instructions.begin();

        for (; it1 != A->send_instructions.end(); it1++)
        {
          if ((*it1)->pid == pid_to_send_info)
          {
            std::list<int>::iterator it2 = std::find( (*it1)->indices.begin(), (*it1)->indices.end(), row );
            if (it2 == (*it1)->indices.end())
            {
              (*it1)->indices.push_back(row);
            }
            send_instruction_posted = true;
            break;
          }
        }

        if (!send_instruction_posted)
        {
          data_transfer_instruction* dti_ptr_1 = new data_transfer_instruction(pid_to_send_info);
          dti_ptr_1->indices.push_back(row);

          A->send_instructions.push_back(dti_ptr_1);
        }
      }
    }
  }
}


void mX_matrix_utils::distributed_sparse_matrix_add_to(distributed_sparse_matrix* M, int row_idx, int col_idx, double val)
{
  // Find the entry or make a new entry using row and column index.
  distributed_sparse_matrix_entry* entry = distributed_sparse_matrix_insert(M, row_idx, col_idx);
  
  // Update the value.
  if (entry)
    entry->value += val;
}


void mX_matrix_utils::sparse_matrix_vector_product(distributed_sparse_matrix* A, distributed_vector &x, distributed_vector &y)
{
  // compute the matrix vector product A*x and return it in y

  // Zero out values in y.
  init_value( y, 0.0 );

  for (int i=0; i<A->n; i++)
  {
    distributed_sparse_matrix_entry* curr = A->row_headers[i];

    while (curr)
    {
      int x_idx, y_idx;

      int col_idx = curr->column;
      double x_value = 0.0;
      std::map<int,int>::iterator it2 = x.row_to_idx.find( col_idx ); 
      if (it2 != x.row_to_idx.end())
      {
        x_value = x.values[it2->second];
        //std::cout << "PID " << A->my_pid << ", col_idx = " << col_idx << ", x_idx = " << it2->second << ", x_values = " << x_value << std::endl;
      } 

      std::map<int,int>::iterator it3 = y.row_to_idx.find( i );
      if (it3 != y.row_to_idx.end())
      {
        y.values[it3->second] += (curr->value)*x_value;
        //std::cout << "PID " << A->my_pid << ", row_idx = " << i << ", y_idx = " << it3->second  << ", y_values = " << y.values[it3->second] << std::endl;
      }

      // Update pointer
      curr = curr->next_in_row;
    }
  }

  // Now assemble vector in parallel before returning.
  mX_vector_utils::assemble_vector( y );

}


void mX_matrix_utils::sparse_matrix_vector_product(distributed_sparse_matrix* A, std::vector<double> &x, std::vector<double> &y)
{
  // compute the matrix vector product A*x and return it in y

#ifdef HAVE_MPI
  // ok, now's the time to follow the send instructions that each pid has been maintaining

  std::list<data_transfer_instruction*>::iterator it1;
  
  for (it1 = A->send_instructions.begin(); it1 != A->send_instructions.end(); it1++)
  {
    std::list<int>::iterator it2;

    for (it2 = (*it1)->indices.begin(); it2 != (*it1)->indices.end(); it2++)
    {
      MPI_Send(&x[*it2],1,MPI_DOUBLE,(*it1)->pid,*it2,MPI_COMM_WORLD);
    }
  }
#endif

  // and everytime a processor receives an x_vec entry
    // it stores the entry in a temporary map

  std::map<int,double> x_vec_entries;
  std::map<int,double>::iterator it3;

  for (int i = 0; i < A->n; i++)
  {
    // compute the mat_vec product for the i'th row
    if (y.size() > i)
    {
      y[i] = (double)(0);
    }
    else
    {
      y.push_back((double)(0));
    }

    distributed_sparse_matrix_entry* curr = A->row_headers[i];

    while(curr)
    {
      int col_idx = curr->column;
      
      if (std::find( A->local_rows.begin(), A->local_rows.end(), col_idx )!= A->local_rows.end())
      {
        // aha, this processor has the correct x_vec entry locally

        y[i] += (curr->value)*x[col_idx];
      }
#ifdef HAVE_MPI
      else
      {
        // this processor does not have the x_vec entry locally
          // but some other processor might have sent it to this guy
            // in which case the entry would have been stored in the local x_vec_entries map
            // so check if the entry is in the map

        it3 = x_vec_entries.find(col_idx);

        if (it3 != x_vec_entries.end())
        {
          y[i] += (double)(it3->second)*(curr->value);
        }
        else
        {
          // no, the entry is not in the map either
            // so this processor waits until someone sends the entry
            // and once it gets the entry, it does two things
              // puts the entry in the map for future reference
              // continues with the matrix vector multiplication

          double x_vec_entry;
          MPI_Status status;
          MPI_Recv(&x_vec_entry,1,MPI_DOUBLE,MPI_ANY_SOURCE,col_idx,MPI_COMM_WORLD,&status);

          x_vec_entries[col_idx] = x_vec_entry;
          y[i] += x_vec_entry*(curr->value);
        }
      }
#endif
      curr = curr->next_in_row;
    }
  }
}

void mX_matrix_utils::destroy_matrix(distributed_sparse_matrix* A)
{
  if (A)
  {
      // delete row_headers
      for (int j=0, cnt=0; j<A->n; ++j, ++cnt)
      {
        distributed_sparse_matrix_entry* curr = (*A).row_headers[cnt], *next = 0;
        
        while (curr)
        {
          next = curr->next_in_row;
          delete curr; curr=0;
          curr = next;
        }
      }
      A->row_headers.resize(0);  

      // delete send_instructions
      while (!A->send_instructions.empty())
      {
        data_transfer_instruction* curr = A->send_instructions.front();
        delete curr; curr=0;
        A->send_instructions.pop_front();
      }

      delete A; A=0;
  }
}

void mX_matrix_utils::print_matrix(distributed_sparse_matrix &A)
{
  if (A.my_pid == 0)
    std::cout << "Matrix\nProc\tRow\tColumn\tValue" << std::endl;

  for (int i=0; i<A.p; ++i)
  {
    if (i == A.my_pid)
    {
      for (int j=0; j<A.n; ++j)
      {
        distributed_sparse_matrix_entry* curr = A.row_headers[j];

        while (curr)
        {
          std::cout << A.my_pid << "\t" << j << "\t" << curr->column << "\t" << curr->value << std::endl;
          curr = curr->next_in_row;
        }
      }
    }
#ifdef HAVE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }
}

void mX_matrix_utils::init_value(distributed_sparse_matrix& A, double val)
{
  for (int j=0; j<A.n; ++j)
  {
    distributed_sparse_matrix_entry* curr = A.row_headers[j];

    while (curr)
    {
      curr->value = val;
      curr = curr->next_in_row;
    }
  }
#ifdef HAVE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}




//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#ifdef HAVE_MPI
#include <mpi.h> // If this routine is compiled with -DHAVE_MPI
                 // then include mpi.h
#endif


#include <iostream>
#include <cmath>

using namespace mX_comm_utils;

void mX_vector_utils::print_vector(distributed_vector& x)
{
  int n = 1, my_pid = 0;

#ifdef HAVE_MPI
  /* Find this processor number */
  MPI_Comm_rank(MPI_COMM_WORLD, &my_pid);

  /* Find the number of processors */
  MPI_Comm_size(MPI_COMM_WORLD, &n);
#endif

  for (int i=0; i<n; ++i)
  {
    if (i == my_pid)
    {
      if (my_pid == 0)
        std::cout << "Proc\tGID\tValue" << std::endl;

      int j=0;
      for ( ; j<x.local_rows.size(); ++j)
      {
        std::cout << my_pid << "\t" << x.local_rows[j] << "\t" << x.values[j] << std::endl;
      }

      for ( ; j<x.row_to_idx.size(); ++j)
      {
        int jj = j-x.local_rows.size();
        std::cout << my_pid << "\t" << x.overlap_rows[jj] << "\t" << x.values[j] << " [O]" << std::endl;
      }

    }
#ifdef HAVE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }
}

double mX_vector_utils::norm2(distributed_vector& x)
{
  double global_norm;
  double local_norm = 0.0;

  for (int i = 0; i < x.local_rows.size(); i++)
  {
    local_norm += x.values[i]*x.values[i];
  }
#ifdef HAVE_MPI
  MPI_Allreduce(&local_norm,&global_norm,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
#else
  global_norm = local_norm;
#endif

  return std::sqrt(global_norm);
}

double mX_vector_utils::dot(distributed_vector& x, distributed_vector& y)
{
  double global_dot;
  double local_dot = 0.0;

  for (int i = 0; i < x.local_rows.size(); i++)
  {
    local_dot += x.values[i]*y.values[i];
  }
#ifdef HAVE_MPI
  MPI_Allreduce(&local_dot,&global_dot,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
#else
  global_dot = local_dot;
#endif

  return global_dot;
}

void mX_vector_utils::assemble_vector(distributed_vector& x)
{
#ifdef HAVE_MPI
  // now's the time to send and receive the results from other processors

  std::list<data_transfer_instruction*>::iterator it1;
  std::list<int>::iterator it2;
  std::map<int,int>::iterator it3;

  for (it1 = x.send_instructions.begin(); it1 != x.send_instructions.end(); it1++)
  {

    for (it2 = (*it1)->indices.begin(); it2 != (*it1)->indices.end(); it2++)
    {
      int x_idx = 0;
      double x_vec_entry = 0.0;
      it3 = x.row_to_idx.find( *it2 );
      if (it3 != x.row_to_idx.end())
      {
        x_vec_entry = x.values[it3->second];
        //std::cout << "PID " << x.my_pid << ", SENDING row_idx = " << *it2 << ", x_idx = " << it3->second << ", x_values = " << x.values[it3->second] << std::endl;
      }

      MPI_Send(&x_vec_entry,1,MPI_DOUBLE,(*it1)->pid,*it2,MPI_COMM_WORLD);
    }
  }
  for (it1 = x.recv_instructions.begin(); it1 != x.recv_instructions.end(); it1++)
  {
    for (it2 = (*it1)->indices.begin(); it2 != (*it1)->indices.end(); it2++)
    {
      int x_idx = 0;
      double x_vec_entry = 0.0;
      MPI_Status status;
      MPI_Recv(&x_vec_entry,1,MPI_DOUBLE,(*it1)->pid,*it2,MPI_COMM_WORLD,&status);

      it3 = x.row_to_idx.find( *it2 );
      if (it3 != x.row_to_idx.end())
      {
        x.values[it3->second] += x_vec_entry;
        //std::cout << "PID " << x.my_pid << ", UPDATED row_idx = " << *it2 << ", x_idx = " << it3->second << ", x_values = " << x.values[it3->second] << std::endl;
      }
   }
 }

#endif
}

void mX_vector_utils::init_value(distributed_vector& x, double val)
{
  for (std::vector<double>::iterator it = x.values.begin(); it != x.values.end(); it++)
  {
    (*it) = val;
  }
} 

void mX_vector_utils::axpy(double alpha, const distributed_vector& x, double beta, distributed_vector& y)
{
  std::vector<double>::const_iterator itx;
  std::vector<double>::iterator ity;
  for (itx = x.values.begin(), ity = y.values.begin(); itx != x.values.end(); itx++, ity++)
  {
    (*ity) = beta*(*ity) + alpha*(*itx);
  }
}

void mX_vector_utils::scale(distributed_vector& x, double val)
{
  for (std::vector<double>::iterator it = x.values.begin(); it != x.values.end(); it++)
  {
    (*it) *= val;
  }
}


//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : Sept 2018

#ifdef HAVE_MPI
#include <mpi.h> // If this routine is compiled with -DHAVE_MPI
                 // then include mpi.h
#endif





#include <map>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>

using namespace mX_comm_utils;
using namespace mX_matrix_utils;
using namespace mX_vector_utils;
using namespace mX_solver_utils;
using namespace mX_DAE_utils;

void mX_solver_utils::gmres(distributed_sparse_matrix* A, distributed_vector &b, distributed_vector &x, double &tol, double &err, int k, int &iters, int &restarts)
{
  // x is expected to contain the initial guess, x0, for the linear system upon entry and will contain the approximate solution upon exit.

  // Define a local k based on the size of the linear system.
  int local_k = std::min( b.n, k );

  std::vector< distributed_vector > V(local_k+1, distributed_vector( b.n, b.p, b.my_pid, b.local_rows, b.overlap_rows, b.send_instructions, b.recv_instructions ) );
  sparse_matrix_vector_product(A,x,V[0]);

  // r0 = temp1 = b - A*x
  axpy( 1.0, b, -1.0, V[0] );

  err = norm2(V[0]);

  int max_restarts = restarts;
  restarts = -1;
  iters = 0;

  std::vector<double> cosines(local_k);
  std::vector<double> sines(local_k);
  std::vector<double> g(local_k+1);

  while ((err > tol) && (restarts < max_restarts))
  {
    // at the start of every re-start
      // the initial guess is already stored in x

    restarts++;
    iters = 0;
    
    // Normalize the first vector, kernel, of the Krylov subspace.
    scale( V[0], 1.0/err );

    std::vector< std::vector<double> > R;
    g[0] = err;

    // ok, Mr.GMRES has determined the initial values for
      // V,R,g,sines,cosines,err and iters
      // he updates these at every iteration until
        // either err becomes less than tol
        // or a new restart is required

    // note that Mr.GMRES does not think it necessary to store the upper Hessenberg matrix at each iteration
      // he computes R at each iteration from the new Hessenberg matrix column
      // and he's clever enough to determine err without having to solve for x at each iteration

    while ((err > tol) && (iters < local_k))
    {  
      iters++;

      // Compute the next Krylov-vector which will require a matrix vector multiplication

      sparse_matrix_vector_product( A, V[iters-1], V[iters] );

      // Perform orthogonalization, modified Gram-Schmidt
      std::vector<double> new_col_H(iters+1);

      for (int i = 0; i < iters; i++)
      {
        double new_dot = dot( V[iters], V[i] );
        axpy( -1.0*new_dot, V[i], 1.0, V[iters] );      
        new_col_H[i] = new_dot;
      }

      double new_norm = norm2( V[iters] );
      new_col_H[iters] = new_norm;
      scale( V[iters], 1.0/new_norm );
     
      // Update the least squares system to get the new column of R using the current sines and cosines

      for (int i = 0; i < iters-1; i++)
      {
        double old_i = new_col_H[i];
        double old_i_plus_one = new_col_H[i+1];

        new_col_H[i] = cosines[i]*old_i + sines[i]*old_i_plus_one;
        new_col_H[i+1] = -sines[i]*old_i + cosines[i]*old_i_plus_one;
      }

      // Compute next Given's rotation
      rotg( new_col_H[iters-1], new_col_H[iters], cosines[iters-1], sines[iters-1] );

      new_col_H.pop_back();
      R.push_back(new_col_H);

      // Right, the new column of R is ready
      // the only thing left to do is to update g
        // which will also tell Mr.GMRES what the new error is

      double old_g = g[iters-1];
      g[iters-1] = old_g*cosines[iters-1];
      g[iters] = -old_g*sines[iters-1];

      err = std::abs(g[iters]);
    }

    // Solve the least-squares system Ry = g
    // after which compute the next solution x += (V without its last column)*y

    std::vector<double> y(iters);

    for (int i = iters-1; i >= 0; i--)
    {
      double sum = (double)(0);

      for (int j = iters-1; j > i; j--)
      {
        sum += R[j][i]*y[j];
      }

      y[i] = (g[i] - sum)/R[i][i];

      // Update the solution.
      axpy( y[i], V[i], 1.0, x );
    }

    // Compute the actual residual 
    sparse_matrix_vector_product(A,x,V[0]);

    // r0 = temp1 = b - A*x
    axpy( 1.0, b, -1.0, V[0] );

    // r0 / || r_0 ||
    double beta = norm2(V[0]);
    err = beta;

    // the new x is also ready
      // either return it or use it as an initial guess for the next restart
  }
  
  // if Mr.GMRES ever reaches here, it means he's solved the problem

  if (restarts < 0)
  {
    restarts = 0;
  }
}

void mX_solver_utils::rotg(double& a, double& b, double& c, double& s)
{
  double roe = b;
  double abs_a = std::abs(a);
  double abs_b = std::abs(b);
  double abs_roe = abs_b;

  if (abs_a > abs_b)
  {
    roe = a;
    abs_roe = abs_a;
  }
  double scale = abs_a + abs_b;
  
  if (scale == 0.0)
  {
    c = 1.0;
    s = 0.0;
  }
  else
  {
    double roe_sign = roe / abs_roe;
    double r = roe_sign*scale*std::sqrt( (a/scale)*(a/scale) + (b/scale)*(b/scale) );
    c = a/r;
    s = b/r;
    double z = 1.0;
    if (abs_a > abs_b)
      z = s;
    if ((abs_b >= abs_a) && (c != 0.0))
      z = 1.0/c;
    a = r;
    b = z;
  }
} 


//@HEADER
// ************************************************************************
// 
//               miniXyce: A simple circuit simulation benchmark code
//                 Copyright (2011) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

// Author : Karthik V Aadithya
// Mentor : Heidi K Thornquist
// Date : July 2010

#include <cctype>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>






#ifdef HAVE_MPI
#include "mpi.h"
#endif

using namespace mX_source_utils;
using namespace mX_matrix_utils;
using namespace mX_DAE_utils;
using namespace mX_device_utils;
  
mX_DAE* mX_parse_utils::parse_netlist(std::string filename, int p, int pid, int &total_devices, int &total_unknowns, int &num_external_nodes, std::map<char, int>& device_count)
{
  std::ifstream infile;
  infile.open(filename.data());
  
  // Make first pass over the netlist and count the number of devices in device_count
  // and collect the integer node names in node_list.
  total_devices = 0;
  total_unknowns = 0;
  device_count.clear();
  std::vector<int> node_list;

  // Make first pass and perform device count
  while (!infile.eof())
  {
    std::string curr_line;
    getline(infile,curr_line);
   
    if ((curr_line.length() == 0) || (curr_line[0] == '*') || (curr_line[0] == '.'))
    {
      continue;  // comments begin with *, ignore '.' lines 
    }

    std::istringstream input_str(curr_line);
    std::string first_word;
    input_str >> first_word;

    // Convert to upper-case.
    std::transform(first_word.begin(), first_word.end(), first_word.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });

    // Increment device count for this device type.
    device_count[first_word[0]]++;
    total_devices++;

    // Save the node names, which are numbers
    mX_device_utils::add_device_nodes( first_word[0], input_str, node_list );

    // Get the number of internal nodes for this device to add to the number of unknowns.
    total_unknowns += mX_device_utils::num_internal_nodes( first_word[0], input_str ); 
  }

  // Sort the node list and count how many nonzero nodes there are.
  std::sort( node_list.begin(), node_list.end() );
  node_list.erase( std::unique( node_list.begin(), node_list.end() ), node_list.end() );
  num_external_nodes = node_list.size();
  if (node_list[0] == 0)
    num_external_nodes--;  // Get rid of ground node, which is '0', from the node count.

  total_unknowns += num_external_nodes;

  // Closing and reopening the file, should be able to rewind the file, but it didn't work
  infile.close();
  infile.open(filename.data());
  //infile.seekg( 0, infile.beg );

  // Variables for the device count
  int voltage_src_number = 0;
  int inductor_number = 0; 

  // Determine how many devices each processor should get.
  int num_my_devices = total_devices/p;
  if ( p > 1 && pid < total_devices%p )
    num_my_devices++;

  std::vector<int> local_dev_count( p, 0 ), global_dev_count( p, 0 );
  local_dev_count[ pid ] = num_my_devices;
  
#ifdef HAVE_MPI
  MPI_Allreduce(&local_dev_count[0],&global_dev_count[0],p,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
#endif

  mX_DAE* dae = new mX_DAE();
  dae->A = new distributed_sparse_matrix();
  dae->B = new distributed_sparse_matrix();

  distributed_sparse_matrix* A = dae->A;
  distributed_sparse_matrix* B = dae->B;

  // initialise the A, B and b parts that need to be stored in this processor
  A->n = total_unknowns;
  A->p = p;
  A->my_pid = pid;

  B->n = total_unknowns; 
  B->p = p;
  B->my_pid = pid;

  for (int i = 0; i < total_unknowns; i++)
  {
    distributed_sparse_matrix_entry* null_ptr_1 = 0;
    A->row_headers.push_back(null_ptr_1);
    B->row_headers.push_back(null_ptr_1);

    mX_DAE_RHS_entry* null_ptr_2 = 0;
    (dae->b).push_back(null_ptr_2);
  }

  // In parallel, processor 0 communicates devices to all other processors
  // before taking its share.
  int current_pid = 0;
  int extra_nodes_ptr = num_external_nodes;

  if (p > 1)
    current_pid = 1;   

  // initialisation done
  // now read each line from the input file and do what is expected
  while (!infile.eof())
  {
    std::string curr_line;
    getline(infile,curr_line);

    if ((curr_line.length() == 0) || (curr_line[0] == '*') || (curr_line[0] == '.'))
    {
      continue;  // comments begin with * 
    }

    std::istringstream input_str(curr_line);
    std::string first_word;
    input_str >> first_word;

    std::transform(first_word.begin(), first_word.end(), first_word.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });

    if ( first_word[0] == 'V' )
        voltage_src_number++;
    if ( first_word[0] == 'L' )
        inductor_number++;

    if (pid == current_pid)
    {
      // Get the container for this device
      std::map<char, mX_device*>::iterator itr = dae->devices.find(first_word[0]);
      mX_device* currDevice = 0;
      if (itr == dae->devices.end())
      {
        //std::cout << "Processor " << current_pid << " didn't find container for device " << first_word[0] << std::endl;
        currDevice = create_device( first_word[0] );
        dae->devices[first_word[0]] = currDevice;
      }
      else
      {
        currDevice = itr->second;
      }

      // If the device generates internal nodes, increment the extra_nodes_ptr.
      extra_nodes_ptr += currDevice->add_device( input_str, extra_nodes_ptr, dae );
    }

    // Update device count and determine if next processor is to receive new device.
    global_dev_count[ current_pid ]--;
    if ( !global_dev_count[ current_pid ] )
    {
      current_pid++;
      // If this is the last processor, change current_pid to processor 0
      if (current_pid == p)
        current_pid = 0;

#ifdef HAVE_MPI
      std::cout << "Processor " << current_pid << " extra_nodes_ptr = " << extra_nodes_ptr << std::endl;
      int my_extra_nodes_ptr = extra_nodes_ptr;
      MPI_Allreduce(&my_extra_nodes_ptr,&extra_nodes_ptr,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);      
#endif
    }
  }

  // Have each device type load their entries now.
  load_matrices( dae );

  // whew! all the lines have been read
    // and each processor hopefully has his correct share of the DAE
  infile.close();

  // Now tell the matrices to assemble themselves and determine off-processor communication.
  distributed_sparse_matrix_finish( A, B, dae->b );

  return dae;
}

#include <ctime>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#ifdef REDSTORM
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

using namespace std;

//set the microapp_name and version which will become part of the YAML doc.
YAML_Doc::YAML_Doc(const std::string& miniApp_Name, const std::string& miniApp_Version, const std::string& destination_Directory, const std::string& destination_FileName){
  miniAppName = miniApp_Name;
  miniAppVersion = miniApp_Version;
  destinationDirectory = destination_Directory;
  destinationFileName = destination_FileName;
}

//inherits the destructor from YAML_Element
YAML_Doc::~YAML_Doc(void){
}

/*
* generates YAML from the elements of the document and saves it
* to a file
*/
string YAML_Doc::generateYAML(){
  string yaml;
  yaml =  yaml + "Mini-Application Name: " + miniAppName + "\n";
  yaml =  yaml + "Mini-Application Version: " + miniAppVersion + "\n";
  for(size_t i=0; i<children.size(); i++){
    yaml = yaml + children[i]->printYAML("");
  }
  
  return yaml;
}



#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
YAML_Element::YAML_Element(const std::string& key_arg, const std::string& value_arg){
  key = key_arg;
  value = value_arg;
}

YAML_Element::~YAML_Element(){
  for (size_t i=0; i<children.size(); i++) {
    delete children[i];
  }
  children.clear();
}

/*
* Add an element to the vector
* QUESTION: if an element is not added because the key already exists,
* will this lead to memory leakage?
*/
YAML_Element* YAML_Element::add(const std::string& key_arg, double value_arg) {
  this->value = "";
  string converted_value = convert_double_to_string(value_arg);
  YAML_Element* element = new YAML_Element(key_arg,converted_value);
  children.push_back(element);
  return element;
}

YAML_Element* YAML_Element::add(const std::string& key_arg, int value_arg) {
  this->value = "";
  string converted_value = convert_int_to_string(value_arg);
  YAML_Element* element = new YAML_Element(key_arg,converted_value);
  children.push_back(element);
  return element;
}

#ifndef MINIFE_NO_LONG_LONG

YAML_Element* YAML_Element::add(const std::string& key_arg, long long value_arg) {
  this->value = "";
  string converted_value = convert_long_long_to_string(value_arg);
  YAML_Element* element = new YAML_Element(key_arg,converted_value);
  children.push_back(element);
  return element;
}

#endif

YAML_Element* YAML_Element::add(const std::string& key_arg, size_t value_arg) {
  this->value = "";
  string converted_value = convert_size_t_to_string(value_arg);
  YAML_Element* element = new YAML_Element(key_arg,converted_value);
  children.push_back(element);
  return element;
}

YAML_Element* YAML_Element::add(const std::string& key_arg, const std::string& value_arg) {
  this->value = "";
  YAML_Element* element = new YAML_Element(key_arg, value_arg);
  children.push_back(element);
  return element;
}

/*
* returns pointer to the YAML_Element for the given key.
* I, cam, believe an exception should be thrown if there is no
* element in the vector for the specified key
*/
YAML_Element* YAML_Element::get(const std::string& key_arg) {
  for (size_t i=0; i<children.size(); i++) {
    if(children[i]->getKey() == key_arg){
      return children[i];
    }
  }
  return 0;
}

/*
* prints a line of a YAML document.  Correct YAML depends on
* correct spacing; the parameter space should be the proper
* amount of space for the parent element
*/
string YAML_Element::printYAML(std::string space){
  string yaml_line = space + key + ": " + value + "\n";
  for(int i=0; i<2; i++) space = space + " ";
  for(size_t i=0; i<children.size(); i++){
    yaml_line = yaml_line + children[i]->printYAML(space);
  }
  return yaml_line;
}

string YAML_Element::convert_double_to_string(double value_arg){
  stringstream strm;
  strm << value_arg;
  return strm.str();
}
string YAML_Element::convert_int_to_string(int value_arg){
  stringstream strm;
  strm << value_arg;
  return strm.str();
}

#ifndef MINIFE_NO_LONG_LONG

string YAML_Element::convert_long_long_to_string(long long value_arg){
  stringstream strm;
  strm << value_arg;
  return strm.str();
}

#endif

string YAML_Element::convert_size_t_to_string(size_t value_arg){
  stringstream strm;
  strm << value_arg;
  return strm.str();
}


double mX_timer() { return 0.0; }
