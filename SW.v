// 1 more cycle, smaller clock period needed ?
module SW #(parameter WIDTH_SCORE = 8, parameter WIDTH_POS_REF = 7, parameter WIDTH_POS_QUERY = 6)
(
    input           clk,
    input           reset,
    input           valid,
    input [1:0]     data_ref,
    input [1:0]     data_query,
    output          finish,
    output [WIDTH_SCORE-1:0]     max,
    output [WIDTH_POS_REF-1:0]   pos_ref,
    output [WIDTH_POS_QUERY-1:0] pos_query
);

//------------------------------------------------------------------
// parameter
//------------------------------------------------------------------
    localparam REF_LENGTH   = 7'd64;
    localparam QUERY_LENGTH = 7'd48;
    localparam PE_NUM       = 7'd16;
    localparam S_IDLE  = 3'd0;
    localparam S_CALC1 = 3'd1;
    localparam S_CALC2 = 3'd2;
    localparam S_CALC3 = 3'd3;
    localparam S_CALC4 = 3'd4;
    localparam S_OUT   = 3'd5;
    integer i;
//------------------------------------------------------------------
// reg & wire
//------------------------------------------------------------------
    reg [1:0] query[0:QUERY_LENGTH-1], query_n[0:QUERY_LENGTH-1];
    reg [1:0] ref[0:REF_LENGTH-1], ref_n[0:REF_LENGTH-1];
    reg [7:0] counter, counter_n;
    reg [5:0] idx, idx_n;
    reg [2:0] state, state_n;
    // PE I/Os
    reg [WIDTH_SCORE-1:0]     row_highest_scores [0:PE_NUM-1], row_highest_scores_n  [0:PE_NUM-1];
    reg [WIDTH_POS_QUERY-1:0] row_highest_columns[0:PE_NUM-1], row_highest_columns_n [0:PE_NUM-1];    
    reg signed  [WIDTH_SCORE-1:0]  PE_align_score_d  [0:PE_NUM-1], PE_align_score_d_n   [0:PE_NUM-1];
    reg signed  [WIDTH_SCORE-1:0]  PE_insert_score_d [0:PE_NUM-1], PE_insert_score_d_n  [0:PE_NUM-1];
    reg signed  [WIDTH_SCORE-1:0]  PE_delete_score_d [0:PE_NUM-1], PE_delete_score_d_n  [0:PE_NUM-1];
    reg signed  [WIDTH_SCORE-1:0]  PE_align_score_dd [0:PE_NUM-1], PE_align_score_dd_n  [0:PE_NUM-1];
    reg signed  [WIDTH_SCORE-1:0]  H_temp [0:QUERY_LENGTH-1], H_temp_n [0:QUERY_LENGTH-1];
    reg signed  [WIDTH_SCORE-1:0]  I_temp [0:QUERY_LENGTH-1], I_temp_n [0:QUERY_LENGTH-1];
    wire signed [WIDTH_SCORE-1:0]  PE_align_score  [0:PE_NUM-1];
    wire signed [WIDTH_SCORE-1:0]  PE_insert_score [0:PE_NUM-1];
    wire signed [WIDTH_SCORE-1:0]  PE_delete_score [0:PE_NUM-1];
    wire signed [WIDTH_SCORE-1:0]  PE_align_diagonal_in; 
    wire                           PE_last_A_base_valid [0:PE_NUM-1];
    wire [1:0]                     PE_last_A_base       [0:PE_NUM-1];
    assign PE_align_diagonal_in = idx > 0 ? H_temp[idx-1] : 8'd0;
    // SW outputs
    reg signed [WIDTH_SCORE-1:0]  highest_score, highest_score_n;
    reg [WIDTH_POS_QUERY-1:0]     column, column_n;
    reg [WIDTH_POS_REF-2:0]       row, row_n;
    assign max       = $unsigned(highest_score);
    assign pos_ref   = {1'b0,row} + 1;
    assign pos_query = column + 1;
    assign finish    = (state == S_OUT) & (counter == 0);
//------------------------------------------------------------------
// submodule
//------------------------------------------------------------------
    genvar gv;
    generate
        for (gv=0; gv<PE_NUM; gv=gv+1) begin: PEs
            if (gv==0) begin
                PE PE_single(
                    .clk                        (clk),
                    .reset                      (reset),

                    .i_A_base_valid             ((state != S_IDLE) & (state != S_OUT) & (idx < QUERY_LENGTH)),
                    .i_A_base                   (query[idx]),
                    .i_B_base                   (ref[gv]),

                    .i_align_top_score          (H_temp[idx]),          //initial : (8'd0),
                    .i_insert_top_score         (I_temp[idx]),          //initial : (-8'd1),
                    .i_align_diagonal_score     (PE_align_diagonal_in), //initial : (8'd0),

                    .i_align_left_score         (PE_align_score_d[gv]),
                    .i_insert_left_score        (PE_insert_score_d[gv]),
                    .i_delete_left_score        (PE_delete_score_d[gv]),

                    .o_align_score              (PE_align_score[gv]),
                    .o_insert_score             (PE_insert_score[gv]),
                    .o_delete_score             (PE_delete_score[gv]),

                    .o_last_A_base_valid        (PE_last_A_base_valid[gv]),
                    .o_last_A_base              (PE_last_A_base[gv])
                );
            end 
            else begin
                PE PE_single(
                    .clk                        (clk),
                    .reset                      (reset),

                    .i_A_base_valid             (PE_last_A_base_valid[gv-1]),
                    .i_A_base                   (PE_last_A_base[gv-1]),
                    .i_B_base                   (ref[gv]),
                    
                    .i_align_diagonal_score     (PE_align_score_dd [gv-1]),
                    .i_align_top_score          (PE_align_score_d  [gv-1]),
                    .i_align_left_score         (PE_align_score_d  [gv]),

                    .i_insert_top_score         (PE_insert_score_d [gv-1]), 
                    .i_insert_left_score        (PE_insert_score_d [gv]),                  
            
                    .i_delete_left_score        (PE_delete_score_d [gv]),

                    .o_align_score              (PE_align_score[gv]),
                    .o_insert_score             (PE_insert_score[gv]),
                    .o_delete_score             (PE_delete_score[gv]),

                    .o_last_A_base_valid        (PE_last_A_base_valid[gv]),
                    .o_last_A_base              (PE_last_A_base[gv])
                );
            end
        end
    endgenerate
//------------------------------------------------------------------
// combinational part
//------------------------------------------------------------------
    // FSM, counter, index
    always@(*) begin
        case(state)
            S_IDLE: begin
                state_n   = valid ? S_CALC1 : state;
                counter_n = counter;
                idx_n     = idx;
            end
            S_CALC1: begin
                state_n   = (counter == QUERY_LENGTH+PE_NUM-2) ? S_CALC2 : state;
                counter_n = counter + 1;
                idx_n     = (counter == QUERY_LENGTH-1) ? 7'd0 : (idx + 1);
            end 
            S_CALC2: begin
                state_n   = (counter == 2*QUERY_LENGTH+PE_NUM-2) ? S_CALC3 : state;
                counter_n = counter + 1;
                idx_n     = (counter == 2*QUERY_LENGTH-1) ? 7'd0 : (idx + 1);
            end   
            S_CALC3: begin
                state_n   = (counter == 3*QUERY_LENGTH+PE_NUM-2) ? S_CALC4 : state;
                counter_n = counter + 1;
                idx_n     = (counter == 3*QUERY_LENGTH-1) ? 7'd0 : (idx + 1);
            end                               
            S_CALC4: begin
                state_n   = (counter == 4*QUERY_LENGTH+PE_NUM-2) ? S_OUT : state;
                counter_n = counter + 1;
                idx_n     = idx + 1;
            end
            S_OUT: begin
                state_n   = (counter == 0) ? S_IDLE : state;
                counter_n = 7'd0;
                idx_n     = 7'd0;
            end
            default: begin
                state_n   = S_IDLE;
                counter_n = 7'd0;
                idx_n     = 7'd0;
            end
        endcase
    end
    // Reference
    always@(*) begin
        if (state == S_IDLE) begin
            for(i=0; i<REF_LENGTH; i=i+1) begin
                if(i == 0) begin
                    ref_n[i] = data_ref;
                end
                else begin
                    ref_n[i] = ref[i];
                end
            end
        end
        else if (counter < QUERY_LENGTH+PE_NUM-1) begin
            for(i=0; i<REF_LENGTH; i=i+1) begin
                if(i == counter+1) begin
                    ref_n[i] = data_ref;
                end
                else if(counter >= QUERY_LENGTH-1 & i == counter-QUERY_LENGTH+1) begin
                    ref_n[i] = ref[i+PE_NUM];
                end
                else begin
                    ref_n[i] = ref[i];
                end
            end
        end
        else if (counter >= 2*QUERY_LENGTH-1 & counter < 2*QUERY_LENGTH+PE_NUM-1) begin
            for(i=0; i<REF_LENGTH; i=i+1) begin
                if(i == counter-2*QUERY_LENGTH+1) begin
                    ref_n[i] = ref[i+2*PE_NUM];
                end
                else begin
                    ref_n[i] = ref[i];
                end
            end
        end
        else if (counter >= 3*QUERY_LENGTH-1 & counter < 3*QUERY_LENGTH+PE_NUM-1) begin
            for(i=0; i<REF_LENGTH; i=i+1) begin
                if(i == counter-3*QUERY_LENGTH+1) begin
                    ref_n[i] = ref[i+3*PE_NUM];
                end
                else begin
                    ref_n[i] = ref[i];
                end
            end
        end
        else begin
            for(i=0; i<REF_LENGTH; i=i+1) begin
                ref_n[i] = ref[i];
            end
        end
    end
    // Query
    always@(*) begin
        case(state)
            S_IDLE: begin
                for(i=0; i<QUERY_LENGTH; i=i+1) begin
                    if(i == 0) begin
                        query_n[i] = data_query;
                    end
                    else begin
                        query_n[i] = query[i];
                    end
                end
            end
            S_CALC1: begin
                for(i=0; i<QUERY_LENGTH; i=i+1) begin
                    if(counter < QUERY_LENGTH-1 & i == counter+1) begin
                        query_n[i] = data_query;
                    end
                    else begin
                        query_n[i] = query[i];
                    end
                end                
            end
            default: begin
                for(i=0; i<QUERY_LENGTH; i=i+1) begin
                    query_n[i] = query[i];
                end                 
            end
        endcase
    end    
    // PE calculation
    always@(*) begin
        case(state)
            S_IDLE: begin
                for (i=0; i<PE_NUM; i=i+1) row_highest_scores_n[i]  = row_highest_scores[i];
                for (i=0; i<PE_NUM; i=i+1) row_highest_columns_n[i] = row_highest_columns[i];
                for (i=0; i<PE_NUM; i=i+1) PE_align_score_d_n[i]    = PE_align_score_d[i];
                for (i=0; i<PE_NUM; i=i+1) PE_insert_score_d_n[i]   = PE_insert_score_d[i];
                for (i=0; i<PE_NUM; i=i+1) PE_delete_score_d_n[i]   = PE_delete_score_d[i];
                for (i=0; i<PE_NUM; i=i+1) PE_align_score_dd_n[i]   = PE_align_score_dd[i];
                for (i=0; i<QUERY_LENGTH; i=i+1) H_temp_n[i] = H_temp[i];
                for (i=0; i<QUERY_LENGTH; i=i+1) I_temp_n[i] = I_temp[i];
            end
            S_CALC1: begin
                for (i=0; i<PE_NUM; i=i+1) row_highest_scores_n[i]  = (PE_align_score[i] > row_highest_scores[i]) ? PE_align_score[i] : row_highest_scores[i];
                for (i=0; i<PE_NUM; i=i+1) row_highest_columns_n[i] = (PE_align_score[i] <= row_highest_scores[i]) ? row_highest_columns[i] :
                                                                      (counter - $unsigned(i) > 8'd48) ? (counter - $unsigned(i) - 8'd48) : (counter - $unsigned(i));
                for (i=0; i<PE_NUM; i=i+1) begin
                    if (i == counter-QUERY_LENGTH+1) begin
                        PE_align_score_d_n[i]  = 8'd0;
                        PE_insert_score_d_n[i] = -8'd1;
                        PE_delete_score_d_n[i] = -8'd1;
                    end
                    else begin
                       PE_align_score_d_n[i]  = PE_align_score[i];
                       PE_insert_score_d_n[i] = PE_insert_score[i];
                       PE_delete_score_d_n[i] = PE_delete_score[i]; 
                    end
                    PE_align_score_dd_n[i] = PE_align_score_d[i];
                end
                for (i=0; i<QUERY_LENGTH; i=i+1) begin
                    if (counter >= PE_NUM-1 & i == counter-(PE_NUM-1)) begin
                        H_temp_n[i] = PE_align_score[PE_NUM-1];
                        I_temp_n[i] = PE_insert_score[PE_NUM-1];
                    end
                    else begin
                        H_temp_n[i] = H_temp[i];
                        I_temp_n[i] = I_temp[i];
                    end
                end
            end
            S_CALC2: begin
                for (i=0; i<PE_NUM; i=i+1) row_highest_scores_n[i]  = (PE_align_score[i] > row_highest_scores[i]) ? PE_align_score[i] : row_highest_scores[i];
                for (i=0; i<PE_NUM; i=i+1) row_highest_columns_n[i] = (PE_align_score[i] <= row_highest_scores[i]) ? row_highest_columns[i] :
                                                                      (counter - $unsigned(i) > 8'd96) ? (counter - $unsigned(i) - 8'd96) : (counter - $unsigned(i) - 8'd48);
                for (i=0; i<PE_NUM; i=i+1) begin
                    if (i == counter-2*QUERY_LENGTH+1) begin
                        PE_align_score_d_n[i]  = 8'd0;
                        PE_insert_score_d_n[i] = -8'd1;
                        PE_delete_score_d_n[i] = -8'd1;
                    end
                    else begin
                       PE_align_score_d_n[i]  = PE_align_score[i];
                       PE_insert_score_d_n[i] = PE_insert_score[i];
                       PE_delete_score_d_n[i] = PE_delete_score[i]; 
                    end
                    PE_align_score_dd_n[i] = PE_align_score_d[i];
                end
                for (i=0; i<QUERY_LENGTH; i=i+1) begin
                    if (i == counter-(QUERY_LENGTH+PE_NUM-1)) begin
                        H_temp_n[i] = PE_align_score[PE_NUM-1];
                        I_temp_n[i] = PE_insert_score[PE_NUM-1];
                    end
                    else begin
                        H_temp_n[i] = H_temp[i];
                        I_temp_n[i] = I_temp[i];
                    end
                end
            end
             S_CALC3: begin
                for (i=0; i<PE_NUM; i=i+1) row_highest_scores_n[i]  = (PE_align_score[i] > row_highest_scores[i]) ? PE_align_score[i] : row_highest_scores[i];
                for (i=0; i<PE_NUM; i=i+1) row_highest_columns_n[i] = (PE_align_score[i] <= row_highest_scores[i]) ? row_highest_columns[i] :
                                                                      (counter - $unsigned(i) > 8'd144) ? (counter - $unsigned(i) - 8'd144) : (counter - $unsigned(i) - 8'd96);
                for (i=0; i<PE_NUM; i=i+1) begin
                    if (i == counter-3*QUERY_LENGTH+1) begin
                        PE_align_score_d_n[i]  = 8'd0;
                        PE_insert_score_d_n[i] = -8'd1;
                        PE_delete_score_d_n[i] = -8'd1;
                    end
                    else begin
                       PE_align_score_d_n[i]  = PE_align_score[i];
                       PE_insert_score_d_n[i] = PE_insert_score[i];
                       PE_delete_score_d_n[i] = PE_delete_score[i]; 
                    end
                    PE_align_score_dd_n[i] = PE_align_score_d[i];
                end
                for (i=0; i<QUERY_LENGTH; i=i+1) begin
                    if (i == counter-(2*QUERY_LENGTH+PE_NUM-1)) begin
                        H_temp_n[i] = PE_align_score[PE_NUM-1];
                        I_temp_n[i] = PE_insert_score[PE_NUM-1];
                    end
                    else begin
                        H_temp_n[i] = H_temp[i];
                        I_temp_n[i] = I_temp[i];
                    end
                end
            end
            S_CALC4: begin
                for (i=0; i<PE_NUM; i=i+1) row_highest_scores_n[i]  = (PE_align_score[i] > row_highest_scores[i]) ? PE_align_score[i] : row_highest_scores[i];
                for (i=0; i<PE_NUM; i=i+1) row_highest_columns_n[i] = (PE_align_score[i] <= row_highest_scores[i]) ? row_highest_columns[i] :
                                                                      (counter - $unsigned(i) > 8'd192) ? (counter - $unsigned(i) - 8'd192) : (counter - $unsigned(i) - 8'd144);
                for (i=0; i<PE_NUM; i=i+1) PE_align_score_d_n[i]    = PE_align_score[i];
                for (i=0; i<PE_NUM; i=i+1) PE_insert_score_d_n[i]   = PE_insert_score[i];
                for (i=0; i<PE_NUM; i=i+1) PE_delete_score_d_n[i]   = PE_delete_score[i];
                for (i=0; i<PE_NUM; i=i+1) PE_align_score_dd_n[i]   = PE_align_score_d[i];
                for (i=0; i<QUERY_LENGTH; i=i+1) H_temp_n[i] = H_temp[i];
                for (i=0; i<QUERY_LENGTH; i=i+1) I_temp_n[i] = I_temp[i];
            end                       
            // S_OUT
            default: begin
                for (i=0; i<PE_NUM; i=i+1) row_highest_scores_n[i]  = row_highest_scores[i];
                for (i=0; i<PE_NUM; i=i+1) row_highest_columns_n[i] = row_highest_columns[i];
                for (i=0; i<PE_NUM; i=i+1) PE_align_score_d_n[i]    = PE_align_score_d[i];
                for (i=0; i<PE_NUM; i=i+1) PE_insert_score_d_n[i]   = PE_insert_score_d[i];
                for (i=0; i<PE_NUM; i=i+1) PE_delete_score_d_n[i]   = PE_delete_score_d[i];
                for (i=0; i<PE_NUM; i=i+1) PE_align_score_dd_n[i]   = PE_align_score_dd[i];
                for (i=0; i<QUERY_LENGTH; i=i+1) H_temp_n[i] = H_temp[i];
                for (i=0; i<QUERY_LENGTH; i=i+1) I_temp_n[i] = I_temp[i];
            end
        endcase
    end
    // PE selecting answer
    always@(*) begin
        if (counter > QUERY_LENGTH-1 & counter < QUERY_LENGTH+PE_NUM) begin
            highest_score_n = (row_highest_scores[counter-QUERY_LENGTH] > highest_score) ? row_highest_scores[counter-QUERY_LENGTH] : highest_score;
            column_n        = (row_highest_scores[counter-QUERY_LENGTH] > highest_score) ? row_highest_columns[counter-QUERY_LENGTH] : column;
            row_n           = (row_highest_scores[counter-QUERY_LENGTH] > highest_score) ? (counter-QUERY_LENGTH) : row;
        end
        else if (counter > 2*QUERY_LENGTH-1 & counter < 2*QUERY_LENGTH+PE_NUM) begin
            highest_score_n = (row_highest_scores[counter-2*QUERY_LENGTH] > highest_score) ? row_highest_scores[counter-2*QUERY_LENGTH] : highest_score;
            column_n        = (row_highest_scores[counter-2*QUERY_LENGTH] > highest_score) ? row_highest_columns[counter-2*QUERY_LENGTH] : column;
            row_n           = (row_highest_scores[counter-2*QUERY_LENGTH] > highest_score) ? (counter-2*QUERY_LENGTH+PE_NUM) : row;               
        end
        else if (counter > 3*QUERY_LENGTH-1 & counter < 3*QUERY_LENGTH+PE_NUM) begin
            highest_score_n = (row_highest_scores[counter-3*QUERY_LENGTH] > highest_score) ? row_highest_scores[counter-3*QUERY_LENGTH] : highest_score;
            column_n        = (row_highest_scores[counter-3*QUERY_LENGTH] > highest_score) ? row_highest_columns[counter-3*QUERY_LENGTH] : column;
            row_n           = (row_highest_scores[counter-3*QUERY_LENGTH] > highest_score) ? (counter-3*QUERY_LENGTH+2*PE_NUM) : row;                
        end
        else if (counter > 4*QUERY_LENGTH-1 & counter < 4*QUERY_LENGTH+PE_NUM) begin
            highest_score_n = (row_highest_scores[counter-4*QUERY_LENGTH] > highest_score) ? row_highest_scores[counter-4*QUERY_LENGTH] : highest_score;
            column_n        = (row_highest_scores[counter-4*QUERY_LENGTH] > highest_score) ? row_highest_columns[counter-4*QUERY_LENGTH] : column;
            row_n           = (row_highest_scores[counter-4*QUERY_LENGTH] > highest_score) ? (counter-4*QUERY_LENGTH+3*PE_NUM) : row;                     
        end
        else begin
            highest_score_n = highest_score;
            column_n        = column;
            row_n           = row;      
        end
    end
//------------------------------------------------------------------
// sequential part
//------------------------------------------------------------------
    always@(posedge clk or posedge reset) begin
        if(reset) begin
            state   <= S_IDLE;
            counter <= 0;
            for (i=0; i<QUERY_LENGTH; i=i+1) query[i] <= 0;
            for (i=0; i<REF_LENGTH; i=i+1)   ref[i]   <= 0;
            // PE
            highest_score <= 0;            
            column        <= 0;
            row           <= 0;
            idx           <= 0;
            for (i=0; i<PE_NUM; i=i+1) row_highest_scores[i]  <= 0;
            for (i=0; i<PE_NUM; i=i+1) row_highest_columns[i] <= 0;
            for (i=0; i<PE_NUM; i=i+1) PE_align_score_d[i]    <= 0;
            for (i=0; i<PE_NUM; i=i+1) PE_insert_score_d[i]   <= -8'd1;
            for (i=0; i<PE_NUM; i=i+1) PE_delete_score_d[i]   <= -8'd1;
            for (i=0; i<PE_NUM; i=i+1) PE_align_score_dd[i]   <= 0;
            for (i=0; i<QUERY_LENGTH; i=i+1) H_temp[i] <= {(WIDTH_SCORE){1'b0}};
            for (i=0; i<QUERY_LENGTH; i=i+1) I_temp[i] <= -8'd1; 
        end
        else begin
            state   <= state_n;
            counter <= counter_n;
            for (i=0; i<QUERY_LENGTH; i=i+1) query[i] <= query_n[i];
            for (i=0; i<REF_LENGTH; i=i+1) ref[i]     <= ref_n[i];
            // PE
            highest_score <= highest_score_n;            
            column        <= column_n;
            row           <= row_n;
            idx           <= idx_n;
            for (i=0; i<PE_NUM; i=i+1) row_highest_scores[i]  <= row_highest_scores_n[i];
            for (i=0; i<PE_NUM; i=i+1) row_highest_columns[i] <= row_highest_columns_n[i];
            for (i=0; i<PE_NUM; i=i+1) PE_align_score_d[i]    <= PE_align_score_d_n[i];
            for (i=0; i<PE_NUM; i=i+1) PE_insert_score_d[i]   <= PE_insert_score_d_n[i];
            for (i=0; i<PE_NUM; i=i+1) PE_delete_score_d[i]   <= PE_delete_score_d_n[i];
            for (i=0; i<PE_NUM; i=i+1) PE_align_score_dd[i]   <= PE_align_score_dd_n[i];
            for (i=0; i<QUERY_LENGTH; i=i+1) H_temp[i] <= H_temp_n[i];
            for (i=0; i<QUERY_LENGTH; i=i+1) I_temp[i] <= I_temp_n[i]; 
        end
    end
    
endmodule
//------------------------------------------------------------------
// PE submodule
//------------------------------------------------------------------
module PE #(parameter WIDTH_SCORE = 8)
(
    input                             clk,
    input                             reset,

    input                             i_A_base_valid,
    input [1:0]                       i_A_base, // reference
    input [1:0]                       i_B_base, // query
    
    input signed  [WIDTH_SCORE-1:0]   i_align_diagonal_score,
    input signed  [WIDTH_SCORE-1:0]   i_align_top_score,
    input signed  [WIDTH_SCORE-1:0]   i_align_left_score, 

    input signed  [WIDTH_SCORE-1:0]   i_insert_top_score,
    input signed  [WIDTH_SCORE-1:0]   i_insert_left_score,
    
    input signed  [WIDTH_SCORE-1:0]   i_delete_left_score,

    output signed [WIDTH_SCORE-1:0]   o_align_score,
    output signed [WIDTH_SCORE-1:0]   o_insert_score,
    output signed [WIDTH_SCORE-1:0]   o_delete_score,

    output                            o_last_A_base_valid,
    output [1:0]                      o_last_A_base
);
    // parameters
    localparam signed match     =  3'd2;
    localparam signed mismatch  = -3'd1;
    localparam signed g_open    =  3'd2;
    localparam signed g_extend  =  3'd1;

    reg signed [2:0] S;
    reg signed [WIDTH_SCORE-1:0] I, D, H;
    reg [1:0] last_A_base;
    reg       last_A_base_valid;
    always@(*) begin
        S = (i_A_base == i_B_base) ? match : mismatch;
    end
    assign o_last_A_base = last_A_base;
    assign o_last_A_base_valid = last_A_base_valid;
    // Insert
    assign o_insert_score = ~i_A_base_valid ? i_insert_left_score :
                            (i_align_top_score - g_open) >= (i_insert_top_score - g_extend) ? (i_align_top_score - g_open) : (i_insert_top_score - g_extend);
    // Delete
    assign o_delete_score = ~i_A_base_valid ? i_delete_left_score :
                            (i_align_left_score - g_open) >= (i_delete_left_score - g_extend) ? (i_align_left_score - g_open) : (i_delete_left_score - g_extend);
    // H(align)
    assign o_align_score  = ~i_A_base_valid ? i_align_left_score :
                            ((i_align_diagonal_score + S) >= o_insert_score & (i_align_diagonal_score + S) >= o_delete_score & (i_align_diagonal_score + S) >= 0) ? (i_align_diagonal_score + S) :
                            (o_insert_score >= (i_align_diagonal_score + S) & o_insert_score >= o_delete_score & o_insert_score >= 0) ? o_insert_score :
                            (o_delete_score >= (i_align_diagonal_score + S) & o_delete_score >= o_insert_score & o_delete_score >= 0) ? o_delete_score : 0;
    // Sequential
    always@(posedge clk or posedge reset) begin
        if (reset) begin
            last_A_base_valid <= 0;
            last_A_base       <= 0;
        end else begin
            last_A_base_valid <= i_A_base_valid;
            last_A_base       <= i_A_base;
        end
    end

endmodule