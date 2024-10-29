//
// Created by fathi on 10/19/2024.
//

#include <stdio.h>
#include "evaluator.h"

/**
 * @brief Evaluates a simple AST node representing binary expressions.
 *
 * This function handles integer and float literals and binary operations
 * such as addition, subtraction, multiplication, and division.
 *
 * @param node Pointer to the ASTNode to evaluate.
 * @return The result of the evaluation as a double.
 */
double evaluate_simple_ast(ASTNode* node) {
    if (node == nullptr) {
        fprintf(stderr, "Error: Null AST node.\n");
        return 0.0;
    }

    switch (node->type) {
        case AST_INTEGER:
            // Cast integer to double for uniformity
            return (double)(node->value->int_value);

        case AST_FLOAT:
            return node->value->float_value;

        case AST_BINARY_OP: {
            // Recursively evaluate left and right operands
            double left = evaluate_simple_ast(node->binary_op_expr.left);
            double right = evaluate_simple_ast(node->binary_op_expr.right);
            TokenType op = node->binary_op_expr.operator;

            // Perform the operation based on the operator type
            switch (op) {
                case TOKEN_PLUS:
                    return left + right;

                case TOKEN_MINUS:
                    return left - right;

                case TOKEN_ASTERISK:
                    return left * right;

                case TOKEN_SLASH:
                    if (right == 0.0) {
                        fprintf(stderr, "Error: Division by zero.\n");
                        return 0.0;
                    }
                    return left / right;

                default:
                    fprintf(stderr, "Error: Unsupported operator %d.\n", op);
                    return 0.0;
            }
        }

        default:
            fprintf(stderr, "Error: Unsupported AST node type %d.\n", node->type);
            return 0.0;
    }
}
